/******************************************************************************
** Copyright(c) 2010 MaK Technologies, Inc.
** All rights reserved.
*******************************************************************************/

#pragma warning(disable: 4786)
#pragma warning(disable: 4290)

#include "NullFederateAmbassador.hh"
#include <map>
#include <string>

class DtTalkAmbData
{
public:
    bool myReceivedInitiateFederateSave;
    bool myReceivedFederationSaved;
    bool myReceivedFederationNotSaved;
    std::string mySaveLabel;

    bool myReceivedRequestFederationRestoreSucceeded;
    bool myReceivedRequestFederationRestoreFailed;
    bool myReceivedFederationRestoreBegun;
    bool myReceivedInitiateFederateRestore;
    bool myReceivedFederationRestored;
    bool myReceivedFederationNotRestored;
    std::string myRestoreLabel;
    RTI::FederateHandle myRestoreFederateHandle;
    std::string myRestoreFailureReason;

    std::map<RTI::ObjectClassHandle, std::string> objectClassMap;
    std::map<RTI::ObjectHandle, std::string> objectInstanceMap;
    std::map<RTI::InteractionClassHandle, std::string> interactionClassMap;
};


class MyFederateAmbassador : public NullFederateAmbassador
{
public:

    MyFederateAmbassador(DtTalkAmbData& data);

    virtual ~MyFederateAmbassador()
        throw (RTI::FederateInternalError);


    ////////////////////////////////////
    // Federation Management Services //
    ////////////////////////////////////

    virtual void initiateFederateSave(
        const char* label) // supplied C4
        throw (
            RTI::UnableToPerformSave,
            RTI::FederateInternalError);

    virtual void federationSaved()
        throw (
            RTI::FederateInternalError);

    virtual void federationNotSaved()
        throw (
            RTI::FederateInternalError);

    virtual void requestFederationRestoreSucceeded(
        const char* label) // supplied C4
        throw (
            RTI::FederateInternalError);

    virtual void requestFederationRestoreFailed(
        const char* label,
        const char* reason) // supplied C4
        throw (
            RTI::FederateInternalError);

    virtual void federationRestoreBegun()
        throw (
            RTI::FederateInternalError);

    virtual void initiateFederateRestore(
        const char* label,   // supplied C4
        RTI::FederateHandle handle)  // supplied C1
        throw (
            RTI::SpecifiedSaveLabelDoesNotExist,
            RTI::CouldNotRestore,
            RTI::FederateInternalError);

    virtual void federationRestored()
        throw (
            RTI::FederateInternalError);

    virtual void federationNotRestored()
        throw (
            RTI::FederateInternalError);


    ////////////////////////////////
    // Object Management Services //
    ////////////////////////////////

    virtual void discoverObjectInstance(
        RTI::ObjectHandle          theObject,      // supplied C1
        RTI::ObjectClassHandle     theObjectClass, // supplied C1
        const char* theObjectName)  // supplied C4  
        throw (
            RTI::CouldNotDiscover,
            RTI::ObjectClassNotKnown,
            RTI::FederateInternalError);

    virtual void reflectAttributeValues(
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
            RTI::FederateInternalError);

    virtual void reflectAttributeValues(
        RTI::ObjectHandle                 theObject,     // supplied C1
        const RTI::AttributeHandleValuePairSet& theAttributes, // supplied C4
        const char* theTag)        // supplied C4
        throw (
            RTI::ObjectNotKnown,
            RTI::AttributeNotKnown,
            RTI::FederateOwnsAttributes,
            RTI::FederateInternalError);

    // 4.6
    virtual void receiveInteraction(
        RTI::InteractionClassHandle       theInteraction, // supplied C1
        const RTI::ParameterHandleValuePairSet& theParameters,  // supplied C4
        const RTI::FedTime& theTime,        // supplied C4
        const char* theTag,         // supplied C4
        RTI::EventRetractionHandle        theHandle)      // supplied C1
        throw (
            RTI::InteractionClassNotKnown,
            RTI::InteractionParameterNotKnown,
            RTI::InvalidFederationTime,
            RTI::FederateInternalError);

    virtual void receiveInteraction(
        RTI::InteractionClassHandle       theInteraction, // supplied C1
        const RTI::ParameterHandleValuePairSet& theParameters,  // supplied C4
        const char* theTag)         // supplied C4
        throw (
            RTI::InteractionClassNotKnown,
            RTI::InteractionParameterNotKnown,
            RTI::FederateInternalError);

    virtual void removeObjectInstance(
        RTI::ObjectHandle          theObject, // supplied C1
        const RTI::FedTime& theTime,   // supplied C4
        const char* theTag,    // supplied C4
        RTI::EventRetractionHandle theHandle) // supplied C1
        throw (
            RTI::ObjectNotKnown,
            RTI::InvalidFederationTime,
            RTI::FederateInternalError);

    virtual void removeObjectInstance(
        RTI::ObjectHandle          theObject, // supplied C1
        const char* theTag)    // supplied C4
        throw (
            RTI::ObjectNotKnown,
            RTI::FederateInternalError);


public:
    DtTalkAmbData& myData;
};
#pragma once
