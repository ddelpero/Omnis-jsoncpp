/* NON-VISUAL OBJECT TEMPLATE (IMPLEMENTATION)
 * 
 * This component is designed to show the basics that are required for creating a non-visual 
 * external object.
 *
 * NOTE: A number of variables in an Omnis external require a "Resource #". This is defined in 
 *       $(ProjectName).rc and, typically, is easiest to edit within a text editor(Not Visual Studio or XCode).
 *
 * By: David McKeone (March 30, 2010)
 * Adapted from the Generic template provided by TigerLogic and omnis.net 
 */

#include "jsoncpp.he"
#include "OmnisTools.he"
#include "Static.he"

#include "JsonValue.he"
#include "JsonReader.he"
#include "JsonWriter.he"
#include "JsonFastWriter.he"

// Resource # of library name
#define LIB_RES_NAME  1000

// Version Information
// 1.0 -- Adapted from Generic
#define VERSION_MAJOR 1
#define VERSION_MINOR 0

// Resources for constants
const static qshort kConstResourcePrefix = 23000,
                    kConstResourceStart  = 23001,
                    kConstResourceEnd    = 23008;


/**************************************************************************************************
 **                             OBJECT DEFINITION                                                **
 **************************************************************************************************/

const static qshort cJsonObjGroup = 1001;

// Resource # for objects.  In this project it is also the Unique ID; when an object is called mCompId will equal this.
const static qshort cJsonValue  = 1002,
                    cJsonReader = 1003,
					cJsonWriter = 1004,
					cJsonFastWriter = 1005;

// Set static id's for matching classes (This is used for creating new objects without needing to know the Omnis ID)
qshort JsonValue::objResourceId = cJsonValue,
       JsonReader::objResourceId = cJsonReader,
	   JsonWriter::objResourceId = cJsonWriter,
	   JsonFastWriter::objResourceId = cJsonFastWriter;

// Omnis objects contained within this component.
// Columns are:
// 1)Unique external ID
// 2)Resource #
// 3)Flags of the type EXTD_FLAG_xxxx
// 4)Group resource ID - The group resource can be passed in here
ECOobject objectsTable[] =
{
	cJsonValue, cJsonValue, 0, cJsonObjGroup,
	cJsonReader, cJsonReader, 0, cJsonObjGroup,
	cJsonWriter, cJsonWriter, 0, cJsonObjGroup,
	cJsonFastWriter, cJsonFastWriter, 0, cJsonObjGroup
};

const qshort cObjCount = sizeof(objectsTable) / sizeof(ECOobject); // Number of Omnis objects in this component

// Return the proper C++ object instance to the message handling method
NVObjBase* createObject( qlong propID, qobjinst objinst, tThreadData *pThreadData ) {
	switch( propID ) {
		case cJsonValue:
			return new JsonValue(objinst, pThreadData);
		case cJsonReader:
			return new JsonReader(objinst, pThreadData);
		case cJsonWriter:
			return new JsonWriter(objinst, pThreadData);
		case cJsonFastWriter:
			return new JsonFastWriter(objinst, pThreadData);
		default:
			return 0;
	}
}

// Copy an Omnis object (and it's associated C++ object) in a type-safe way
void copyObject( qlong propID, objCopyInfo* copyInfo, tThreadData *pThreadData ) {
    using OmnisTools::copyNVObj;
    
    switch (propID) {
        case cJsonValue:
            copyNVObj<JsonValue>(propID, copyInfo, pThreadData);
            break;
        case cJsonReader:
            copyNVObj<JsonReader>(propID, copyInfo, pThreadData);
            break;
        case cJsonWriter:
            copyNVObj<JsonWriter>(propID, copyInfo, pThreadData);
            break;
		case cJsonFastWriter:
            copyNVObj<JsonFastWriter>(propID, copyInfo, pThreadData);
            break;
        default:
            break;
    }
}

// Delete the proper C++ object instance
void removeObject( qlong propID, NVObjBase* nvObj ) {
	switch( propID ) {
		case cJsonValue:
			delete dynamic_cast<JsonValue*>(nvObj);
			break;
		case cJsonReader:
			delete dynamic_cast<JsonReader*>(nvObj);
			break;
		case cJsonWriter:
			delete dynamic_cast<JsonWriter*>(nvObj);
			break;
		case cJsonFastWriter:
			delete dynamic_cast<JsonFastWriter*>(nvObj);
			break;
		default:
			break;
	}
	return;
}

/**************************************************************************************************
 **                                    MAIN ENTRY POINT                                          **
 **************************************************************************************************/

// Component library entry point (name as declared in resource 31000)
// This method is used as a dispatch point for all behaviour in the component.
// All messages(Msg) that are not caught in the switch() are handled by WNDdefWindowProc()

extern "C" qlong OMNISWNDPROC NVObjWndProc(HWND hwnd, LPARAM Msg, WPARAM wParam, LPARAM lParam, EXTCompInfo* eci)
{
	ECOsetupCallbacks(hwnd, eci); // Initialize callback tables - THIS MUST BE DONE 		
	
	// Message Dispatch
	switch (Msg)
	{		
            // ECM_OBJCONSTRUCT - this is a message to create a new object.
		case ECM_OBJCONSTRUCT:				
		{
		    // Check if object already exists (if it does there is nothing more to do)
			NVObjBase* nvObj = (NVObjBase*) ECOfindNVObject(eci->mOmnisInstance, lParam );
			if ( nvObj ) { return qtrue; }
			
		    // Build thread data
			tThreadData threadData(eci);
			
		    // Allocate a new object
			qlong propID = eci->mCompId; // Get ID of object
			nvObj = createObject(propID, (qobjinst)lParam, &threadData );
			if (nvObj) {
                // and insert into a chain of objects. The OMNIS library will maintain this chain
				ECOinsertNVObject( eci->mOmnisInstance, lParam, nvObj );
				return qtrue;
			}
			return qfalse;
		}    
			
            // ECM_OBJDESTRUCT - this is a message to inform you to delete the object
		case ECM_OBJDESTRUCT:					
		{			
			if (ECM_WPARAM_OBJINFO == wParam) // Make sure its when the NV object is really meant to go (as opposed to $destruct being called)
			{
			    // First find the object in the libraries chain of objects, 
				// this call, if ok, also removes the object from the chain.
				void* obj = ECOremoveNVObject( eci->mOmnisInstance, lParam );
				if ( obj )
				{ 
					// Now you can delete the object you previously allocated.
				    // Note: The hwnd passed on ECM_OBJCONSTRUCT should not be deleted, as
				    // it was created (and will be destroyed by) OMNIS
					qlong propID = eci->mCompId; // Get ID of object
					removeObject(propID, (NVObjBase*) obj);
				}
			}
			return qtrue;
		}
			
            // ECM_CONNECT - this message is sent once per OMNIS session and should not be confused 
            // with ECM_OBJCONSTRUCT which is sent once per object. ECM_CONNECT can be used to load other libraries
            // once or other general global actions that need to be done only once.
            //
            // For most components this can be removed - see other BLYTH component examples
		case ECM_CONNECT:
		{
			return EXT_FLAG_LOADED|EXT_FLAG_NVOBJECTS|EXT_FLAG_REMAINLOADED|EXT_FLAG_ALWAYS_USABLE; // Return external flags. Loaded & Has Non-Visual Objects
		} 
			
            // ECM_DISCONNECT - this message is sent only once when the OMNIS session is ending and should not be confused
            // with ECM_OBJDESTRUCT which is sent once per object. ECM_DISCONNECT can be used to free other libraries
            // loaded on ECM_CONNECT or other general global actions that need to be done only once.
            // 
            // For most components this can be removed - see other BLYTH component examples
		case ECM_DISCONNECT:
		{ 
			return qtrue;
		}
			
            // ECM_OBJECT_COPY - this message is called when an object needs to be copied by Omnis (and you must do so in C++ as well).
		case ECM_OBJECT_COPY:
		{
			tThreadData threadData(eci);
			objCopyInfo* copyInfo = (objCopyInfo*)lParam;
			qlong propID = eci->mCompId;  // Get ID of object
			
            copyObject(propID, copyInfo, &threadData);
		}
			
            // ECM_GETSTATICOBJECT - this is sent by OMNIS to retrieve a list of static methods
		case ECM_GETSTATICOBJECT:
		{
			tThreadData threadData(eci);
			return returnStaticMethods( &threadData );
		}
			
            // ECM_GETMETHODNAME - this is sent by OMNIS to retrieve a list of methods for object instances
		case ECM_GETMETHODNAME:
		{			
			tThreadData threadData(eci);
			qlong objID = eci->mCompId;  // Get ID of object
			
			// Create a temporary object, use it to retrieve the methods, and destruct it.
			NVObjBase* obj = createObject( objID, 0, &threadData ); // 0 = no qobjinst (because this isn't being created by Omnis)
			qlong ret = obj->returnMethods( &threadData );  // Call virtual method to get properties for the requested object
			removeObject( objID, obj );
			
			return ret;
		}
			
            // ECM_METHODCALL - this is sent by OMNIS to call a method in the non-visual object
		case ECM_METHODCALL:
		{
			tThreadData threadData(eci);
			
			void* obj = ECOfindNVObject( eci->mOmnisInstance, lParam );
			if ( NULL != obj )
			{ 
				// Method from Object Instance
				NVObjBase* nvObj = reinterpret_cast<NVObjBase*>(obj);
				return nvObj->methodCall(&threadData);
			} else {
				// Static method
				return staticMethodCall( &threadData );
			}
			
			return qfalse;
		}
			
			
            // ECM_GETPROPNAME - this is sent by OMNIS to find out which properties an object has
		case ECM_GETPROPNAME:
		{
			tThreadData threadData(eci);
			qlong objID = eci->mCompId; // Get ID of object
			
		    // Create a temporary object, use it to retrieve the properties, and destruct it.
			NVObjBase* obj = createObject( objID, 0, &threadData );  // 0 = no qobjinst (because this isn't being created by Omnis)
			qlong ret = obj->returnProperties( &threadData );  // Call virtual method to get properties for the requested object
			removeObject( objID, obj );
			
			return ret;
		}
			
			
            // ECM_PROPERTYCANASSIGN - this is sent by OMNIS to find out if a specific property can be assigned or not
		case ECM_PROPERTYCANASSIGN:
		{
		    tThreadData threadData(eci);
			
			// Get ID of property
			qlong propID = ECOgetId( eci );
			
		    // Get the instance of the object
			NVObjBase* nvObj = reinterpret_cast<NVObjBase*>(ECOfindNVObject(eci->mOmnisInstance, lParam));
			if( NULL != nvObj )
				// Get the property from that instance
				return nvObj->canAssignProperty( &threadData, propID );
			else
				return qfalse;
		}
			
            // ECM_GETPROPERTY - this is sent by OMNIS to determine the value of a specific property
		case ECM_GETPROPERTY:
		{
			tThreadData threadData(eci);
			
			// Get the instance of the object
			NVObjBase* nvObj = reinterpret_cast<NVObjBase*>(ECOfindNVObject(eci->mOmnisInstance, lParam));
			if( NULL != nvObj )
				
				// Get the property from that instance
				return nvObj->getProperty( &threadData );
			else
				return 0L;
		}
			
            // ECM_SETPROPERTY - this is sent by OMNIS to set the value of a specific property
		case ECM_SETPROPERTY:
		{			
			tThreadData threadData(eci);
			
			NVObjBase* nvObj = reinterpret_cast<NVObjBase*>(ECOfindNVObject(eci->mOmnisInstance, lParam));
			if( NULL != nvObj )
                // Set the property in the instance
				return nvObj->setProperty( &threadData );
			else
				return 0L;
		}
			
            // ECM_GETCONSTPREFIX - this is sent by OMNIS to get the prefix that is used for all constants in this component
		case ECM_CONSTPREFIX:
		{
			EXTfldval exfldval;
			str80 conPrefix; 
			
			// Read string from resource, and assign it to return parameter
			RESloadString(gInstLib,kConstResourcePrefix,conPrefix);
			exfldval.setChar(conPrefix);
			
			ECOaddParam(eci,&exfldval);
			return qtrue;
		}
			
            // ECM_GETCONSTNAME - this is sent by OMNIS to get all the names of the constants in the component
		case ECM_GETCONSTNAME:
		{
			return ECOreturnConstants( gInstLib, eci, kConstResourceStart, kConstResourceEnd );
		}
			
            // ECM_GETCOMPLIBINFO - this is sent by OMNIS to find out the name of the library, and
            // the number of visual components this library supports
		case ECM_GETCOMPLIBINFO:
		{
			return ECOreturnCompInfo( gInstLib, eci, LIB_RES_NAME, 0 );
		}
			
            // ECM_GETOBJECT - this is sent by OMNIS to find out which Omnis objects this component is responsible for
		case ECM_GETOBJECT:
		{
			return ECOreturnObjects( gInstLib, eci, &objectsTable[0], cObjCount );
		}
			
            // ECM_GETVERSION - this is sent by OMNIS to find out the version of your external
		case ECM_GETVERSION:
		{
			return ECOreturnVersion(VERSION_MAJOR,VERSION_MINOR);
		}
			
            // ECM_ISUNICODE - this is sent by OMNIS to find out if your external supports unicode
		case ECM_ISUNICODE:
		{
#ifdef UNICODE
			return qtrue;  // This external is Studio 5.0+ only
#else
			return qfalse;
#endif
		}
	}
	
	// As a final result this must ALWAYS be called. It handles all other messages that this component
	// decides to ignore.
	return WNDdefWindowProc(hwnd,Msg,wParam,lParam,eci);
}
