/*******************************************************************************
** Copyright (c) 2004 MaK Technologies, Inc.
** All rights reserved.
*******************************************************************************/

#pragma warning(disable: 4786)
#pragma warning(disable: 4290)

#include <stdio.h>
#include <iostream>
#include "hla_amb.h"

using namespace std;

void printAttributes(const RTI::AttributeHandleValuePairSet& attributes)
{
    for (unsigned int iloop = 0; iloop < attributes.size(); iloop++)
    {
        RTI::ULong length = attributes.getValueLength(iloop);
        cout << attributes.getHandle(iloop) << " "
            << attributes.getValuePointer(iloop, length) << endl;
    }
}

void printParameters(const RTI::ParameterHandleValuePairSet& params)
{
    for (unsigned int iloop = 0; iloop < params.size(); iloop++)
    {
        RTI::ULong length = params.getValueLength(iloop);
        cout << params.getHandle(iloop) << " "
            << params.getValuePointer(iloop, length) << endl;
    }
}

std::string timeToString(RTI::FedTime const& time)
{
    char* buff = new char[time.getPrintableLength() + 1];
    ((RTI::FedTime&)time).getPrintableString(buff);
    std::string timeStr(buff);
    delete[] buff;
    return timeStr;
}

MyFederateAmbassador::MyFederateAmbassador(DtTalkAmbData& data) :
    NullFederateAmbassador(), myData(data)
{
}

MyFederateAmbassador::~MyFederateAmbassador()
throw (RTI::FederateInternalError)
{
    int x = 1;
}

void MyFederateAmbassador::initiateFederateSave(const char* label)
throw (
    RTI::UnableToPerformSave,
    RTI::FederateInternalError)
{
    cout << "initiateFederateSave: "
        << label << endl;
    myData.myReceivedInitiateFederateSave = true;
    myData.mySaveLabel = label;
}

void MyFederateAmbassador::federationSaved()
throw (RTI::FederateInternalError)
{
    cout << "federationSaved " << endl;
    myData.myReceivedFederationSaved = true;
}

void MyFederateAmbassador::federationNotSaved()
throw (RTI::FederateInternalError)
{
    cout << "federationNotSaved: ";
    myData.myReceivedFederationNotSaved = true;
}

void MyFederateAmbassador::requestFederationRestoreSucceeded(const char* label)
throw (RTI::FederateInternalError)
{
    cout << "requestFederationRestoreSucceeded "
        << label << endl;
    myData.myReceivedRequestFederationRestoreSucceeded = true;
    myData.myRestoreLabel = label;
}

void MyFederateAmbassador::requestFederationRestoreFailed(const char* label,
    const char* reason)
    throw (RTI::FederateInternalError)
{
    cout << "requestFederationRestoreFailed "
        << label << " " << reason << endl;
    myData.myReceivedRequestFederationRestoreFailed = true;
    myData.myRestoreLabel = label;
    myData.myRestoreFailureReason = reason;
}

void MyFederateAmbassador::federationRestoreBegun()
throw (RTI::FederateInternalError)
{
    cout << "federationRestoreBegun" << endl;
    myData.myReceivedFederationRestoreBegun = true;
}

void MyFederateAmbassador::initiateFederateRestore(
    const char* label,
    RTI::FederateHandle handle)
    throw (
        RTI::SpecifiedSaveLabelDoesNotExist,
        RTI::CouldNotRestore,
        RTI::FederateInternalError)
{
    cout << "initiateFederateRestore "
        << label << " " << handle << endl;
    myData.myReceivedInitiateFederateRestore = true;
    myData.myRestoreLabel = label;
    myData.myRestoreFederateHandle = handle;
}

void MyFederateAmbassador::federationRestored()
throw (RTI::FederateInternalError)
{
    cout << "federationRestored" << endl;
    myData.myReceivedFederationRestored = true;
}

void MyFederateAmbassador::federationNotRestored()
throw (RTI::FederateInternalError)
{
    cout << "federationNotRestored" << endl;
    myData.myReceivedFederationNotRestored = true;
}

void MyFederateAmbassador::discoverObjectInstance(
    RTI::ObjectHandle theObject,
    RTI::ObjectClassHandle theObjectClass,
    const char* theObjectName)
    throw (
        RTI::CouldNotDiscover,
        RTI::ObjectClassNotKnown,
        RTI::FederateInternalError)
{
    cout << "discoverObjectInstance: "
        << theObjectName << "("
        << theObject << ") of class "
        << myData.objectClassMap[theObjectClass].c_str() << "( "
        << theObjectClass << ")" << endl; \
        myData.objectInstanceMap[theObject] = theObjectName;
}

void MyFederateAmbassador::reflectAttributeValues(
    RTI::ObjectHandle                 theObject,     // supplied C1
    const RTI::AttributeHandleValuePairSet& theAttributes, // supplied C4
    const RTI::FedTime& theTime,       // supplied C1
    const char* theTag,        // supplied C4
    RTI::EventRetractionHandle        theHandle)     // supplied C1
    throw (
        RTI::ObjectNotKnown,
        RTI::AttributeNotKnown,
        RTI::FederateOwnsAttributes,
        RTI::InvalidFederationTime,
        RTI::FederateInternalError)
{
    cout << "reflectAttributeValues: "
        << myData.objectInstanceMap[theObject].c_str() << "("
        << theObject << ") "
        << timeToString(theTime).c_str() << " "
        << (theTag ? theTag : "") << " "
        << "#attributes: " << theAttributes.size() << endl;
    printAttributes(theAttributes);
}

void MyFederateAmbassador::reflectAttributeValues(
    RTI::ObjectHandle                 theObject,     // supplied C1
    const RTI::AttributeHandleValuePairSet& theAttributes, // supplied C4
    const char* theTag)        // supplied C4
    throw (
        RTI::ObjectNotKnown,
        RTI::AttributeNotKnown,
        RTI::FederateOwnsAttributes,
        RTI::FederateInternalError)
{
    cout << "reflectAttributeValues: "
        << myData.objectInstanceMap[theObject].c_str() << "("
        << theObject << ") "
        << (theTag ? theTag : "") << " "
        << "#attributes: " << theAttributes.size() << endl;
    printAttributes(theAttributes);
}

// 4.6
void MyFederateAmbassador::receiveInteraction(
    RTI::InteractionClassHandle       theInteraction, // supplied C1
    const RTI::ParameterHandleValuePairSet& theParameters,  // supplied C4
    const RTI::FedTime& theTime,        // supplied C4
    const char* theTag,         // supplied C4
    RTI::EventRetractionHandle        theHandle)      // supplied C1
    throw (
        RTI::InteractionClassNotKnown,
        RTI::InteractionParameterNotKnown,
        RTI::InvalidFederationTime,
        RTI::FederateInternalError)
{
    cout << "receiveInteraction: "
        << myData.interactionClassMap[theInteraction].c_str() << "( "
        << theInteraction << ") "
        << timeToString(theTime).c_str() << " "
        << (theTag ? theTag : "") << " "
        << "#parameters: " << theParameters.size() << endl;
    printParameters(theParameters);
}

void MyFederateAmbassador::receiveInteraction(
    RTI::InteractionClassHandle       theInteraction, // supplied C1
    const RTI::ParameterHandleValuePairSet& theParameters,  // supplied C4
    const char* theTag)         // supplied C4
    throw (
        RTI::InteractionClassNotKnown,
        RTI::InteractionParameterNotKnown,
        RTI::FederateInternalError)
{
    cout << "receiveInteraction: "
        << myData.interactionClassMap[theInteraction].c_str() << "( "
        << theInteraction << ") "
        << (theTag ? theTag : "") << " "
        << "#parameters: " << theParameters.size() << endl;
    printParameters(theParameters);
}

void MyFederateAmbassador::removeObjectInstance(
    RTI::ObjectHandle          theObject, // supplied C1
    const RTI::FedTime& theTime,   // supplied C4
    const char* theTag,    // supplied C4
    RTI::EventRetractionHandle theHandle) // supplied C1
    throw (
        RTI::ObjectNotKnown,
        RTI::InvalidFederationTime,
        RTI::FederateInternalError)
{
    cout << "removeObjectInstance: "
        << myData.objectInstanceMap[theObject].c_str() << "("
        << theObject << ") "
        << timeToString(theTime).c_str() << " "
        << (theTag ? theTag : "") << endl;
}

void MyFederateAmbassador::removeObjectInstance(
    RTI::ObjectHandle          theObject, // supplied C1
    const char* theTag)    // supplied C4
    throw (
        RTI::ObjectNotKnown,
        RTI::FederateInternalError)
{
    cout << "removeObjectInstance: "
        << myData.objectInstanceMap[theObject].c_str() << "("
        << theObject << ") "
        << (theTag ? theTag : "") << endl;
}
