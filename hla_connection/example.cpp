#pragma warning(disable: 4786)
#pragma warning(disable: 4290)
#pragma warning(disable: 4996)


#include <winsock2.h>
#include <process.h>
#include <stdio.h>
#include <cstdlib>
#include <cstring>

#include <map>
#include <string>
#include <iostream>
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

		for (int i = 0; i < 20; i++)
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

			}
		}

		
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
void joinFedEx(RTI::RTIambassador& rtiAmb, MyFederateAmbassador* fedAmb, string const& federateType, string const& federationName) {
	bool joined = false;
	const int maxTry = 10;
	int numTries = 0;

	cout << "joinFederationExecution "
		<< federateType.c_str() << " "
		<< federationName.c_str() << endl;

	while (!joined && numTries++ < maxTry) {
		try
		{
			theFederateHandle = rtiAmb.joinFederationExecution(federateType.c_str(), federationName.c_str, fedAmb);
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
	}
	rtiAmb.tick(0.1, 0.2);
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
		cnt =  1;
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
