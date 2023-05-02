#pragma warning(disable: 4786)
#pragma warning(disable: 4290)

#define RTI_USES_STD_FSTREAM

#include <winsock2.h>
#include <process.h>
#include <stdio.h>
#include <cstdlib>
#include <cstring>

#include <map>
#include <string>
#include <iostream>
#include <ostream>
#include <sstream>

#include "RTI.hh"
#include "hla_amb.h"

using namespace std;

// Map between strings and attribute handles
typedef std::map<std::string, RTI::AttributeHandle>  DtAttrNameHandleMap;

// Map between strings and parameter handles
typedef std::map<std::string, RTI::ParameterHandle>  DtParamNameHandleMap;

// The object class to be retreived from RTI
RTI::ObjectClassHandle theClassHandle;

RTI::ObjectHandle theObjectHandle;

RTI::FederateHandle theFederateHandle;

RTI::InteractionClassHandle theInterClassHandle;

string theClassName = "BaseEntity";
string theInterClassName = "WeaponFire";

DtTalkAmbData theAmbData;

DtAttrNameHandleMap theAttrNameHandleMap;
DtParamNameHandleMap theParamNameHandleMap;

void createFedEx(RTI::RTIambassador& rtiAmb, string const& fedName, string const& fedFile);
bool publishSubscribeAndRegisterObject(RTI::RTIambassador& rtiAmb);
void joinFedEx(RTI::RTIambassador& rtiAmb, MyFederateAmbassador* fedAmb, std::string const& federateType, std::string const& federationName);
void resignAndDestroy(RTI::RTIambassador& rtiAmb, std::string const& federationName);
bool publishAndSubscribeInteraction(RTI::RTIambassador& rtiAmb);
bool saveFederation(RTI::RTIambassador& rtiAmb, string const& label, bool performRequest);
bool requestFederationSave(RTI::RTIambassador& rtiAmb, string const& label);
bool waitForFederationSaved(RTI::RTIambassador& rtiAmb);
bool restoreFederation(RTI::RTIambassador& rtiAmb, string const& label, bool performRequest);
bool requestFederationRestore(RTI::RTIambassador& rtiAmb, std::string const& label);
bool waitForRestoreBegun(RTI::RTIambassador& rtiAmb);
bool waitForInitiateRestore(RTI::RTIambassador& rtiAmb);
bool waitForFederationRestored(RTI::RTIambassador& rtiAmb);

int main() {

	try {
		string federationName("MAKsimple");
		string federationFile("MAKsimple.fed");
		string federateType("rtisimple13");

		RTI::RTIambassador rtiAmb;
		MyFederateAmbassador fedAmb(theAmbData);

		RTI::AttributeHandleValuePairSet* attrValues = 0;
		RTI::ParameterHandleValuePairSet* paramValues = 0;

		rtiAmb.tick();
		long count = 0;
		bool doConnect = true;
		bool connected = false;

		for (int i = 0; i < 200; i++)
		{
			if (doConnect) {
				doConnect = false;

				createFedEx(rtiAmb, federationName, federationFile);
				joinFedEx(rtiAmb, &fedAmb, federateType, federationName);

				if (!publishSubscribeAndRegisterObject(rtiAmb)) {
					resignAndDestroy(rtiAmb, federationName);
					return 0;
				}
				if (!publishAndSubscribeInteraction(rtiAmb)) {
					resignAndDestroy(rtiAmb, federationName);
				}

				attrValues = RTI::AttributeSetFactory::create(theAttrNameHandleMap.size());

				for (DtAttrNameHandleMap::iterator iter = theAttrNameHandleMap.begin(); iter != theAttrNameHandleMap.end(); iter++) {
					attrValues->add(iter->second, iter->first.c_str(), iter->first.length() + 1);
				}

				paramValues = RTI::ParameterSetFactory::create(theParamNameHandleMap.size());

				for (DtParamNameHandleMap::iterator iter2 = theParamNameHandleMap.begin(); iter2 != theParamNameHandleMap.end(); iter2++) {
					paramValues->add(iter2->second, iter2->first.c_str(), iter2->first.length() + 1);
				}

				connected = true;
			}
			else if (connected) {
				stringstream ss;
				ss << "1.3-" << count++;
				string tag(ss.str());

				if (count % 5 == 0) {
					cout << "Sending interaction..." << endl;
					rtiAmb.sendInteraction(
						theInterClassHandle,
						*paramValues,
						tag.c_str());
				}

				rtiAmb.tick(0.1, 0.5);

				if (theAmbData.myReceivedInitiateFederateSave) {
					saveFederation(rtiAmb, theAmbData.mySaveLabel, false);
				}

				if (theAmbData.myReceivedFederationRestoreBegun) {
					restoreFederation(rtiAmb, theAmbData.myRestoreLabel, false);
				}
			}
		}

		Sleep(2000);

		resignAndDestroy(rtiAmb, federationName);
	}
	catch (RTI::Exception& ex) {
		cout << "RTI Exception (main loop): "
			<< ex._name << " "
			<< ex._reason << endl;
	}
	return 0;
}
void createFedEx(RTI::RTIambassador& rtiAmb, string const& fedName, string const& fedFile) {
	try {
		rtiAmb.createFederationExecution(fedName.c_str(), fedFile.c_str());
	}
	catch (RTI::FederationExecutionAlreadyExists& ex) {
		cout << "Could not create Federation Execution: "
			<< "FederationExecutionAlreadyExists: "
			<< ex._name << " "
			<< ex._reason << endl;
	}
	catch (RTI::Exception& ex) {
		cout << "Could not create Federation Execution: " << endl
			<< "RTI Exception: "
			<< ex._name << " "
			<< ex._reason << endl;
		exit(0);
	}
	rtiAmb.tick(0.1, 0.2);

	cout << "Federation created" << endl;
}
bool publishSubscribeAndRegisterObject(RTI::RTIambassador& rtiAmb) {
	try
	{
		theClassHandle = rtiAmb.getObjectClassHandle(theClassName.c_str());
		theAmbData.objectClassMap[theClassHandle] = theClassName;
	}
	catch (RTI::Exception& ex)
	{
		cout << "RTI Exception: "
			<< ex._name << " "
			<< ex._reason << endl
			<< "Could not get object class handle: "
			<< theClassName.c_str() << endl;
		return false;
	}
	string attrName;

	try
	{
		attrName = "AccelerationVector";
		theAttrNameHandleMap[attrName] =
			rtiAmb.getAttributeHandle(attrName.c_str(), theClassHandle);
		attrName = "VelocityVector";
		theAttrNameHandleMap[attrName] =
			rtiAmb.getAttributeHandle(attrName.c_str(), theClassHandle);
		attrName = "Orientation";
		theAttrNameHandleMap[attrName] =
			rtiAmb.getAttributeHandle(attrName.c_str(), theClassHandle);
		attrName = "DeadReckoningAlgorithm";
		theAttrNameHandleMap[attrName] =
			rtiAmb.getAttributeHandle(attrName.c_str(), theClassHandle);
		attrName = "WorldLocation";
		theAttrNameHandleMap[attrName] =
			rtiAmb.getAttributeHandle(attrName.c_str(), theClassHandle);
	}
	catch (RTI::Exception& ex)
	{
		cout << "RTI Exception: "
			<< ex._name << " "
			<< ex._reason << endl
			<< "Could not get attribute handle "
			<< attrName.c_str() << endl;
		return false;
	}

	RTI::AttributeHandleSet* hSet = RTI::AttributeHandleSetFactory::create(theAttrNameHandleMap.size());

	for (DtAttrNameHandleMap::iterator iter = theAttrNameHandleMap.begin(); iter != theAttrNameHandleMap.end(); iter++) {
		hSet->add(iter->second);
	}

	int cnt = 0;

	try
	{
		rtiAmb.publishObjectClass(theClassHandle, *hSet);
		rtiAmb.tick(0.1, 0.2);
		cnt = 1;
		rtiAmb.subscribeObjectClassAttributes(theClassHandle, *hSet);
		rtiAmb.tick(0.1, 0.2);

	}
	catch (RTI::Exception& ex)
	{
		cout << "RTI Exception: "
			<< ex._name << " "
			<< ex._reason << endl
			<< "Could not "
			<< (cnt ? "publish" : "subscribe") << endl;
		delete hSet;
		return false;
	}

	string objectName("Talk");

	try
	{
		unsigned int objId = abs(_getpid());
		stringstream pid;
		pid << objId;
		objectName += pid.str();
		theObjectHandle = rtiAmb.registerObjectInstance(theClassHandle, objectName.c_str());

		//Add name-handle to map
		theAmbData.objectInstanceMap[theObjectHandle] = rtiAmb.getObjectInstanceName(theObjectHandle);
		rtiAmb.tick(0.1, 0.2);
	}
	catch (RTI::Exception& ex)
	{
		cout << "RTI Exception: "
			<< ex._name << " "
			<< ex._reason << endl
			<< "Could not  Register Object "
			<< objectName.c_str()
			<< " with class "
			<< theClassName.c_str() << endl;
		delete hSet;
		return false;
	}

	cout << "Registered object "
		<< objectName.c_str()
		<< " with class name "
		<< theClassName.c_str() << endl;
	delete hSet;
	return true;
}
////////////////////////////////////////////////////////////////////////////////
// Join the federation execution
void joinFedEx(RTI::RTIambassador& rtiAmb, MyFederateAmbassador* fedAmb, std::string const& federateType, std::string const& federationName)
{
	bool joined = false;
	const int maxTry = 10;
	int numTries = 0;
	cout << "joinFederationExecution "
		<< federateType.c_str() << " "
		<< federationName.c_str() << endl;

	while (!joined && numTries++ < maxTry)
	{
		try
		{
			theFederateHandle = rtiAmb.joinFederationExecution(federateType.c_str(),
				federationName.c_str(), fedAmb);
			joined = true;
		}
		catch (RTI::FederationExecutionDoesNotExist)
		{
			cout << "FederationExecutionDoesNotExist, try "
				<< numTries << "out of "
				<< maxTry << endl;
			continue;
		}
		catch (RTI::Exception& ex)
		{
			cout << "RTI Exception: "
				<< ex._name << " "
				<< ex._reason << endl;
			return;
		}
		rtiAmb.tick(0.1, 0.2);
	}
	if (joined)
		cout << "Joined Federation." << endl;
	else
	{
		cout << "Giving up." << endl;
		rtiAmb.destroyFederationExecution(federationName.c_str());
		exit(0);
	}
}

////////////////////////////////////////////////////////////////////////////////
// Resign and destroy the federation execution
void resignAndDestroy(RTI::RTIambassador& rtiAmb, std::string const& federationName)
{
	cout << "Resign and Destroy Federation" << endl;
	rtiAmb.resignFederationExecution(RTI::DELETE_OBJECTS);
	rtiAmb.destroyFederationExecution(federationName.c_str());
}

bool publishAndSubscribeInteraction(RTI::RTIambassador& rtiAmb)
{
	try
	{
		theInterClassHandle = rtiAmb.getInteractionClassHandle(theInterClassName.c_str());
		theAmbData.interactionClassMap[theInterClassHandle] = theInterClassHandle;
	}
	catch (RTI::Exception& ex) {
		cout << "RTI Exception: "
			<< ex._name << " "
			<< ex._reason << endl
			<< "Could not get interaction class handle: "
			<< theInterClassName.c_str() << endl;
		return false;
	}

	string paramName;

	try
	{
		paramName = "EventIdentifier";
		theParamNameHandleMap[paramName] =
			rtiAmb.getParameterHandle(paramName.c_str(), theInterClassHandle);
		paramName = "FireControlSolutionRange";
		theParamNameHandleMap[paramName] =
			rtiAmb.getParameterHandle(paramName.c_str(), theInterClassHandle);
		paramName = "FireMissionIndex";
		theParamNameHandleMap[paramName] =
			rtiAmb.getParameterHandle(paramName.c_str(), theInterClassHandle);
		paramName = "FiringLocation";
		theParamNameHandleMap[paramName] =
			rtiAmb.getParameterHandle(paramName.c_str(), theInterClassHandle);
		paramName = "FiringObjectIdentifier";
		theParamNameHandleMap[paramName] =
			rtiAmb.getParameterHandle(paramName.c_str(), theInterClassHandle);
		paramName = "FuseType";
		theParamNameHandleMap[paramName] =
			rtiAmb.getParameterHandle(paramName.c_str(), theInterClassHandle);
		paramName = "InitialVelocityVector";
		theParamNameHandleMap[paramName] =
			rtiAmb.getParameterHandle(paramName.c_str(), theInterClassHandle);
		paramName = "MunitionObjectIdentifier";
		theParamNameHandleMap[paramName] =
			rtiAmb.getParameterHandle(paramName.c_str(), theInterClassHandle);
		paramName = "MunitionType";
		theParamNameHandleMap[paramName] =
			rtiAmb.getParameterHandle(paramName.c_str(), theInterClassHandle);
		paramName = "QuantityFired";
		theParamNameHandleMap[paramName] =
			rtiAmb.getParameterHandle(paramName.c_str(), theInterClassHandle);
		paramName = "RateOfFire";
		theParamNameHandleMap[paramName] =
			rtiAmb.getParameterHandle(paramName.c_str(), theInterClassHandle);
		paramName = "TargetObjectIdentifier";
		theParamNameHandleMap[paramName] =
			rtiAmb.getParameterHandle(paramName.c_str(), theInterClassHandle);
		paramName = "WarheadType";
		theParamNameHandleMap[paramName] =
			rtiAmb.getParameterHandle(paramName.c_str(), theInterClassHandle);
	}
	catch (RTI::Exception& ex)
	{
		cout << "RTI Exception: "
			<< ex._name << " "
			<< ex._reason << endl
			<< "Could not get parameter handle "
			<< paramName.c_str() << endl;
		return false;
	}

	int cnt = 0;

	try
	{
		rtiAmb.publishInteractionClass(theInterClassHandle);
		rtiAmb.tick(0.1, 0.2);
		cnt = 1;
		rtiAmb.subscribeInteractionClass(theInterClassHandle);
		rtiAmb.tick(0.1, 0.2);
	}
	catch (RTI::Exception& ex)
	{
		cout << "RTI Exception: "
			<< ex._name << " "
			<< ex._reason << endl
			<< "Could not "
			<< (cnt ? "publish" : "subscribe")
			<< " to interaction." << endl;
		return false;
	}
	cout << "Subscribed to interaction class: "
		<< theInterClassName.c_str()
		<< " with handle: "
		<< theInterClassHandle << endl;
	return true;
}

bool saveFederation(RTI::RTIambassador& rtiAmb, string const& label, bool performRequest) {
	bool saveOk = true;
	int count = 0;

	try {
		if (performRequest) {
			saveOk = requestFederationSave(rtiAmb, label);
		}

		if (theAmbData.myReceivedInitiateFederateSave) {
			// At this point, the save has been initiated
			// and the federate cannot invoke any other calls except
			// to complete the save (or resign). It signals to the RTI that
			// it will begin saving its own local state

			cout << "Federate save begun\n";
			rtiAmb.federateSaveBegun();

			// Here the federate would save its own state including any context
			// information relating to the RTI
			// (i.e. what classes are published and subscribed
			// object instance handles for registered and discovered objects, etc.

			// Once the federate saves is own data, it signals to the RTI that its
			// save is complete.
			cout << "Federate save complete\n";
			rtiAmb.federateSaveComplete();

			// Wait until the RTI signals that the entire federation completed the save
			saveOk = waitForFederationSaved(rtiAmb);
		}
	}
	catch (RTI::Exception& ex)
	{
		cout << "RTI Exception: "
			<< ex._name << " "
			<< ex._reason << endl;
		cout << "Could not  save federation " << label.c_str() << endl;
		saveOk = false;
	}
	catch (exception& stdEx)
	{
		cout << "Standard exception: " << stdEx.what() << endl;
		cout << "Could not  save federation " << label.c_str() << endl;
		saveOk = false;
	}
	catch (...)
	{
		cout << "Unknown exception\n";
		cout << "Could not  save federation " << label.c_str() << endl;
		saveOk = false;
	}

	// Turn the signals off
	theAmbData.myReceivedInitiateFederateSave =
		theAmbData.myReceivedFederationSaved =
		theAmbData.myReceivedFederationNotSaved = false;

	return saveOk;
}

bool requestFederationSave(RTI::RTIambassador& rtiAmb, string const& label) {
	bool saveOK = true;
	int count = 0;

	theAmbData.myReceivedInitiateFederateSave = theAmbData.myReceivedFederationSaved = theAmbData.myReceivedFederationNotSaved = false;

	cout << "Request federation save with label " << label.c_str() << endl;

	// Request the save and wait for the signal that the save has been initiated
	// or that the federation has not saved (could be that some other federate
	// resigns during save)
	rtiAmb.requestFederationSave(label.c_str());

	while (count++ < 100
		&& !theAmbData.myReceivedInitiateFederateSave
		&& !theAmbData.myReceivedFederationNotSaved)
	{
		rtiAmb.tick(0.1, 0.2);
	}

	if (!theAmbData.myReceivedInitiateFederateSave)
	{
		saveOK = false;
		if (!theAmbData.myReceivedFederationNotSaved)
		{
			cout << "Timed out waiting for initiate federate save\n";
		}
	}
	return saveOK;
}

bool waitForFederationSaved(RTI::RTIambassador& rtiAmb) {
	bool saveOK = true;
	int count = 0;

	while (count++ < 100 && !theAmbData.myReceivedFederationNotSaved && !theAmbData.myReceivedFederationSaved) {
		rtiAmb.tick(0.1, 0.2);
	}

	if (!theAmbData.myReceivedFederationSaved) {
		saveOK = false;
		if (!theAmbData.myReceivedFederationNotSaved) {
			cout << "Timed out waiting for federation saved\n";
		}
	}
	return saveOK;
}

bool restoreFederation(RTI::RTIambassador& rtiAmb, string const& label, bool performRequest) {
	bool restoreOk = true;
	int count = 0;

	try
	{
		if (performRequest)
		{
			if (requestFederationRestore(rtiAmb, label))
			{
				restoreOk = waitForRestoreBegun(rtiAmb);
			}
		}

		if (theAmbData.myReceivedFederationRestoreBegun)
		{
			// At this point, the restore has begun
			// and the federate cannot invoke any other calls except
			// to complete the restore (or resign). It waits for the
			// initiate restore to begin restoring its local state.

			if (restoreOk = waitForInitiateRestore(rtiAmb))
			{
				// Here the federate would restore its own state including any context
				// information relating to the RTI
				// (i.e. what classes are published and subscribed
				// object instance handles for registered and discovered objects, etc.

				// Once the federate restores its own data, it signals to the RTI that its
				// restore is complete.
				wcout << L"Federate restore complete\n";
				rtiAmb.federateRestoreComplete();
				// If the federate was unable to restore its state, it would respond with
				// federateRestoreNotComplete();

				// Now wait for remaining federation to be restored
				restoreOk = waitForFederationRestored(rtiAmb);
			}
		}
	}
	catch (RTI::Exception& ex)
	{
		cout << "RTI Exception: "
			<< ex._name << " "
			<< ex._reason << endl;
		cout << "Could not  save federation " << label.c_str() << endl;
		restoreOk = false;
	}
	catch (exception& stdEx)
	{
		cout << "Standard exception: " << stdEx.what() << endl;
		cout << "Could not  save federation " << label.c_str() << endl;
		restoreOk = false;
	}
	catch (...)
	{
		cout << "Unknown exception\n";
		cout << "Could not  save federation " << label.c_str() << endl;
		restoreOk = false;
	}

	// Turn the signals off
	theAmbData.myReceivedRequestFederationRestoreSucceeded =
		theAmbData.myReceivedRequestFederationRestoreFailed =
		theAmbData.myReceivedFederationRestoreBegun =
		theAmbData.myReceivedInitiateFederateRestore =
		theAmbData.myReceivedFederationRestored =
		theAmbData.myReceivedFederationNotRestored = false;

	return restoreOk;
}

bool requestFederationRestore(RTI::RTIambassador& rtiAmb, std::string const& label)
{
	int count = 0;
	bool restoreOk = true;

	cout << "Request federation restore with label " << label.c_str() << endl;

	// Turn the signals off
	theAmbData.myReceivedRequestFederationRestoreSucceeded =
		theAmbData.myReceivedRequestFederationRestoreFailed =
		theAmbData.myReceivedFederationRestoreBegun =
		theAmbData.myReceivedInitiateFederateRestore =
		theAmbData.myReceivedFederationRestored =
		theAmbData.myReceivedFederationNotRestored = false;

	rtiAmb.requestFederationRestore(label.c_str());
	while (count++ < 100
		&& !theAmbData.myReceivedRequestFederationRestoreSucceeded
		&& !theAmbData.myReceivedRequestFederationRestoreFailed)
	{
		rtiAmb.tick(0.1, 0.2);
	}

	if (!theAmbData.myReceivedRequestFederationRestoreSucceeded)
	{
		restoreOk = false;
		if (!theAmbData.myReceivedRequestFederationRestoreFailed)
		{
			cout << "Timed out waiting for request federation restore succeeded.\n";
		}
	}
	return restoreOk;
}

// Wait for restore begun
// Return begun status
bool waitForRestoreBegun(RTI::RTIambassador & rtiAmb)
{
	bool restoreOk = true;
	// Wait for the restore begun

	cout << "Wait for restore begun\n";
	int count = 0;
	while (count++ < 100
		&& !theAmbData.myReceivedFederationRestoreBegun
		&& !theAmbData.myReceivedFederationNotRestored)
	{
		rtiAmb.tick(0.1, 0.2);
	}

	if (!theAmbData.myReceivedFederationRestoreBegun)
	{
		restoreOk = false;
		if (!theAmbData.myReceivedFederationNotRestored)
		{
			cout << "Timed out waiting for federation restore begun.\n";
		}
	}
	return restoreOk;
}

// Wait for initiate restore
// Return initiate status
bool waitForInitiateRestore(RTI::RTIambassador& rtiAmb)
{
	bool restoreOk = true;

	// Wait for the initiate restore

	cout << "Wait for initiate restore\n";

	int count = 0;
	while (count++ < 100
		&& !theAmbData.myReceivedInitiateFederateRestore
		&& !theAmbData.myReceivedFederationNotRestored)
	{
		rtiAmb.tick(0.1, 0.2);
	}

	if (theAmbData.myReceivedFederationNotRestored)
	{
		restoreOk = false;
	}
	else if (!theAmbData.myReceivedInitiateFederateRestore)
	{
		restoreOk = false;
		cout << "Timed out waiting for initiate federate restore.\n";
	}
	else if (theAmbData.myRestoreFederateHandle != theFederateHandle)
	{
		// In general, a federate must be able to restore any saved state of
		// a federate of the same type. Even if a federate submits
		// a unique federate type string during join, the current federate
		// and object handles may not match those being restored.
		// This simplistic federate implementation cannot handle the case where
		// the federate and object handles are different.
		cout << "Restore initiated with handle " << theAmbData.myRestoreFederateHandle
			<< " does not match current handle " << theFederateHandle << endl;
		cout << "Unable to complete restore\n";

		restoreOk = false;

		// Will revert to state before restore was begun
		rtiAmb.federateRestoreNotComplete();

		count = 0;
		while (count++ < 100
			&& !theAmbData.myReceivedFederationNotRestored)
		{
			rtiAmb.tick(0.1, 0.2);
		}

		if (!theAmbData.myReceivedFederationNotRestored)
		{
			cout << "Timed out waiting for federation not restored.\n";
		}
	}

	return restoreOk;
}

bool waitForFederationRestored(RTI::RTIambassador& rtiAmb)
{
	bool restoreOk = true;
	// Wait until the RTI signals that the entire federation completed the restore

	cout << "Wait for federation restored\n";
	int count = 0;
	while (count++ < 100
		&& !theAmbData.myReceivedFederationRestored
		&& !theAmbData.myReceivedFederationNotRestored)
	{
		rtiAmb.tick(0.1, 0.2);
	}

	if (!theAmbData.myReceivedFederationRestored)
	{
		restoreOk = false;
		if (!theAmbData.myReceivedFederationNotRestored)
		{
			cout << "Timed out waiting for federation restored.\n";
		}
	}
	return restoreOk;
}

