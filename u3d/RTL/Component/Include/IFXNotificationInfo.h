//***************************************************************************
//
//  Copyright (c) 1999 - 2006 Intel Corporation
//
//  Licensed under the Apache License, Version 2.0 (the "License");
//  you may not use this file except in compliance with the License.
//  You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
//  Unless required by applicable law or agreed to in writing, software
//  distributed under the License is distributed on an "AS IS" BASIS,
//  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
//  See the License for the specific language governing permissions and
//  limitations under the License.
//
//***************************************************************************
//
//	IFXNotificationInfo.h
//
//	DESCRIPTION
//		This module defines the IFXNotificationInfo class.  It is used to...
//
//	NOTES
//
//***************************************************************************

/**
	@file	IFXNotificationInfo.h

			This header file defines the IFXNotificationInfo interface and its functionality.
*/

#ifndef __IFXNotificationInfo_H__
#define __IFXNotificationInfo_H__


#include "IFXUnknown.h"
#include "IFXTaskData.h"
#include "IFXString.h"


/**
	The set of notification types recognized by the IFXNotificationManager 
	interface.

	Notification types are like event categories, and help filter the mass of events
	There are a small number of system-defined notification types.

	Clients defining their own custom notification types should obtain them
	at run-time by calling IFXNotificationManager::GetNextType() to ensure
	there will be no collisions. Such dynamically allocated types will have
	values higher than IFXNotificationType_Auto.

	If it is necessary to define custom notification types at compile time,
	values should have the general form (IFXNotificationType_User + n) and
	be less than IFXNotificationType_Auto.

	Note that while auto-generated types are guaranteed to be unique,
	types defined at compile time relative to IFXNotificationType_User
	could possibly collide with types defined by another plug-in component,
	and for that reason are not recommended.

	@note	IFXNotificationType_User is the default user type for use with 
			user-defined notification ids. Plug-ins can generate their own 
			custom notification types.

	@verbatim
	Predefined system notification types: 0x00000000-0x07FFFFFF   (134,217,727 unique ids)
	User definable notification types:    0x08000000-0x0FFFFFFF   (134,217,727 unique ids)
	Autogenerated notification types:     0x10000000-0xFFFFFFFF (4,026,531,839 unique ids)
	@endverbatim

	@todo	Some of these notification type are place holders and have not 
			been hooked up yet.  Many also require better descriptions.
*/
enum IFXNotificationType
{
	/**
		Indicates no type or wildcard behavior when registering to receive 
		notifications.
	*/
	IFXNotificationType_Nil =  0,
	IFXNotificationType_Animation, // affected=model
	IFXNotificationType_Collision, // affected=model, userdata=collisiondata


	/**
		Notification generated when a task execution fails (submission is 
		done via IFXNotificationManager::SubmitError).  Notification id 
		contains the IFXTaskHandle of the registered being monitored or 
		IFXNotificationId_Nil to monitor all registered tasks.  Key filter 
		contains the IFXRESULT result code returned by the failing task.  
		Object filter contains a reference to the failing task.
	*/
	IFXNotificationType_Error,
	IFXNotificationType_Idle,      // affected=user, id=user
	IFXNotificationType_Input,

	/**
		System type.  Usage depends upon the associated IFXNotificationId.
	*/
	IFXNotificationType_System,
	IFXNotificationType_Task,      // affected=task
	IFXNotificationType_Time,      // affected=user, id=time, userdata=details
	IFXNotificationType_Update,    // affected=object, id=attribute, userdata=details
	IFXNotificationType_Script,    // reserved

	IFXNotificationType_User = 0x08000000,	// affected=user, id=user
	IFXNotificationType_Auto = 0x10000000,	// affected=user, id=user
	IFXNotificationType_Invalid = 0xFFFFFFFF
};

/**
	The set of notification ids recognized by the IFXNotificationManager.

	Notification ids are unique ids that identify each possible event type. It is
	possible to combine Type and Id to create a unique 64-bit identifier.

	Clients defining their own custom notification IDs should obtain them
	at run-time by calling IFXNotificationManager::GetNextId() to ensure
	there will be no collisions. Such dynamically allocated IDs will have
	values higher than IFXNotificationId_Auto.

	If it is necessary to define custom notification IDs at compile time,
	values should have the general form (IFXNotificationId_User + n) and
	be less than IFXNotificationId_Auto.

	It is not necessary for IDs to be unique globally, as long as they are
	unique within a type.

	@todo	Many of these notification ids are place holders for clients and 
			are unused, or have not been hooked up yet.  Many also require 
			better descriptions.

	@note	

	There are three general categories of Id: predefined, autogenerated, and user.

	Predefined Ids are defined in this header file, although others may exist
	internally. Autogenerated ids are served up by the GetNextId method in the
	NotificationManager. User ids are reserved for use by plug-in developers who
	need fixed ids.

	It is recommended that you use autogenerated Ids when possible to avoid
	conflicts. Autogenerated ids are given the largest range, and there is room
	to expand the range in the future. It is also strongly recommended that any
	custom ids are defined relative to these defines, as their specific values
	may change; however, the available user-id space will not shrink, so you
	may freely use the entire range.

	@verbatim
	Predefined system notification ids: 0x00000000-0x07FFFFFF   (134,217,727 unique ids)
	User definable notification ids:    0x08000000-0x0FFFFFFF   (134,217,727 unique ids)
	Autogenerated notification ids:     0x10000000-0xFFFFFFFF (4,026,531,839 unique ids)
	@endverbatim
*/
enum IFXNotificationId
{
	IFXNotificationId_Nil = 0,				///< Indicates no id or wildcard behavior when registering to receive notifications.

	// IFXNotificationType_Animation
	IFXNotificationId_AnimationQueued,		///< Id used with type IFXNotificationType_Animation.
	IFXNotificationId_AnimationStarted,		///< Id used with type IFXNotificationType_Animation.
	IFXNotificationId_AnimationEnded,		///< Id used with type IFXNotificationType_Animation.

	// IFXNotificationType_Collision
	IFXNotificationId_CollisionDetected,	///< Id used with type IFXNotificationType_Collision.
	IFXNotificationId_CollisionDetectedBone,///< Id used with type IFXNotificationType_Collision.
	IFXNotificationId_CollisionResolved,	///< Id used with type IFXNotificationType_Collision.

	// IFXNotificationType_Input
	IFXNotificationId_MouseDown,			///< Id used with type IFXNotificationType_Input.
	IFXNotificationId_MouseUp,				///< Id used with type IFXNotificationType_Input.
	IFXNotificationId_MouseDrag,			///< Id used with type IFXNotificationType_Input.
	IFXNotificationId_KeyDown,				///< Id used with type IFXNotificationType_Input.
	IFXNotificationId_KeyUp,				///< Id used with type IFXNotificationType_Input.

	// IFXNotificationType_System
	IFXNotificationId_ClockStarted,			///< Id used with type IFXNotificationType_System.
	IFXNotificationId_ClockStopped,			///< Id used with type IFXNotificationType_System.
	IFXNotificationId_ClockChanged,			///< Id used with type IFXNotificationType_System.
	IFXNotificationId_UnknownBlockSkipped,	///< Id used with type IFXNotificationType_System.  Notifiction data contains the unrecognized U32 block type, not a void pointer.  Key filter contains the associated U32 load id.

	// IFXNotificationType_Task
	IFXNotificationId_TaskRegistered,		///< Id used with type IFXNotificationType_Task.
	IFXNotificationId_TaskUnregistered,		///< Id used with type IFXNotificationType_Task.
	IFXNotificationId_TaskReset,			///< Id used with type IFXNotificationType_Task.

	// IFXNotificationType_Update
	IFXNotificationId_StreamState,			///< Id used with type IFXNotificationType_Update.

	// IFXNotificationType_User
	IFXNotificationId_User = 0x08000000,	///< Id used with type IFXNotificationType_User.
	IFXNotificationId_Auto = 0x10000000,	///< Id used with type IFXNotificationType_User.
	IFXNotificationId_Invalid = 0xFFFFFFFF
};

//-------------------------------------------------------------------
// Interface
//-------------------------------------------------------------------

/**
	This is the main interface for IFXNotificationInfo.\n

	IFXNotificationInfo extends the IFXTaskData filtering mechanism with user definable fields.\n

	IFXNotificationType serves as a high-level filter. A task registered as type X will only
	handle events of type X.
	However, a task registered as type IFXNotificationType_Nil will match the type of any event.\n

	IFXNotificationId works similarly to IFXNotificationType, including the wildcard behavior for
	IFXNotificationId_Nil.	In some cases IFXNotificationId is treated as a variable rather than
	a predefined type. 	For example, errors	(IFXNotificationType_Error) set the Id field to an
	IFXRESULT which identifies the error; tasks therefore can register to handle any error or
	a specific error by setting their pattern Id appropriately.\n

	Keyfilter is a simple user-defined integer which may be used as a filter. The value must match
	exactly. Zero is a wildcard.\n

	BitFilter is a bitfield. It's value is bitwise ANDed with the pattern, and if the result
	is nonzero, it is a match. Zero is a wildcard.\n

	ObjectFilter is an IFXUnknown object. The pointers are compared, and if both are the same
	object, it is a match. NULL is a wildcard. This is intended primarily as a refcounted
	alternative to UserData.\n

	Name is simply a string that may be associated with the event. It could be a name or
	it could be descriptive. It is not used in filtering.\n

	See IFXTaskData for more information on the general pattern matching scheme for events.\n

	@note	The associated IID is named IID_IFXNotificationInfo,
	and the CID for the component is CID_IFXNotificationInfo.
*/

class IFXNotificationInfo : public IFXTaskData
{
public:
	/**
		This method sets the Notification Data for this component.
		The notification data is a user pointer with no restrictions on its value.

		@param	pPointer	The notification data pointer.\n

		@return	Returns an IFXRESULT code.\n
		- @b IFX_OK upon success.\n
		- @b IFX_E_NOT_INITIALIZED if the IFXNotificationInfo has not been initialized.\n
	*/
	virtual IFXRESULT IFXAPI  SetNotificationData(void * pPointer) = 0;

	/**
		This method retrieves the Notification Data for this component.
		If not set, the pointer defaults to NULL.

		@param	ppPointer	Receives the notification data pointer.\n

		@return	Returns an IFXRESULT code.\n
		- @b IFX_OK upon success.\n
		- @b IFX_E_NOT_INITIALIZED if the IFXNotificationInfo has not been initialized.\n
		- @b IFX_E_INVALID_POINTER is returned if ppPointer is NULL.\n
	*/
	virtual IFXRESULT IFXAPI  GetNotificationData(void ** ppPointer) = 0;

	// events have a type and an id

	/**
		This method sets the IFXNotificationType for this component.

		@param	type	The IFXNotificationType.\n

		@return	Returns an IFXRESULT code.\n
		- @b IFX_OK upon success.\n
		- @b IFX_E_NOT_INITIALIZED if the IFXNotificationInfo has not been initialized.\n
	*/
	virtual IFXRESULT IFXAPI  SetType(IFXNotificationType type) = 0;

	/**
		This method retrieves the IFXNotificationType for this component.

		@param	pType	Receives the IFXNotificationType.\n

		@return	Returns an IFXRESULT code.\n
		- @b IFX_OK upon success.\n
		- @b IFX_E_NOT_INITIALIZED if the component has not been initialized.\n
		- @b IFX_E_INVALID_POINTER is returned if pType is NULL.\n
	*/
	virtual IFXRESULT IFXAPI  GetType(IFXNotificationType * pType) = 0;

	/**
		This method sets the IFXNotificationId for this component.

		@param	id	The IFXNotificationId.\n

		@return	Returns an IFXRESULT code.\n
		- @b IFX_OK upon success.\n
		- @b IFX_E_NOT_INITIALIZED if the component has not been initialized.\n
	*/
	virtual IFXRESULT IFXAPI  SetId(IFXNotificationId id) = 0;

	/**
		This method retrieves the IFXNotificationId for this component.

		@param	pId	Receives the IFXNotificationId.\n

		@return	Returns an IFXRESULT code.\n
		- @b IFX_OK upon success.\n
		- @b IFX_E_NOT_INITIALIZED if the component has not been initialized.\n
		- @b IFX_E_INVALID_POINTER is returned if pId is NULL.\n
	*/
	virtual IFXRESULT IFXAPI  GetId(IFXNotificationId * pId) = 0;

	// events can have an associated name

	/**
		This method sets the associated name for this notification event.

		@param	name	The event name.\n

		@return	Returns an IFXRESULT code.\n
		- @b IFX_OK upon success.\n
		- @b IFX_E_NOT_INITIALIZED if the component has not been initialized.\n
	*/
	virtual IFXRESULT IFXAPI  SetName(IFXString name) = 0;

	/**
		This method retrieves the associated name for this notification event.

		@param	name	Receives the event name.\n

		@return	Returns an IFXRESULT code.\n
		- @b IFX_OK upon success.\n
		- @b IFX_E_NOT_INITIALIZED if the component has not been initialized.\n
	*/
	virtual IFXRESULT IFXAPI  GetName(IFXString &name) = 0;

	// filtering

	/**
		This method sets the keyFilter for this component.

		@param	keyFilter	The keyFilter.\n

		@return	Returns an IFXRESULT code.\n
		- @b IFX_OK upon success.\n
		- @b IFX_E_NOT_INITIALIZED if the component has not been initialized.\n
	*/
	virtual IFXRESULT IFXAPI  SetKeyFilter(U32 keyFilter) = 0;

	/**
		This method retrieves the keyFilter for this component.

		@param	pKeyFilter	Receives the keyFilter.\n

		@return	Returns an IFXRESULT code.\n
		- @b IFX_OK upon success.\n
		- @b IFX_E_NOT_INITIALIZED if the component has not been initialized.\n
		- @b IFX_E_INVALID_POINTER is returned if pKeyFilter is NULL.\n
	*/
	virtual IFXRESULT IFXAPI  GetKeyFilter(U32 *pKeyFilter) = 0;

	/**
		This method sets the bitFilter for this component.

		@param	bitFilter	The bitFilter.\n

		@return	Returns an IFXRESULT code.\n
		- @b IFX_OK upon success.\n
		- @b IFX_E_NOT_INITIALIZED if the component has not been initialized.\n
	*/
	virtual IFXRESULT IFXAPI  SetBitFilter(U32 bitFilter) = 0;

	/**
		This method retrieves the bitFilter for this component.

		@param	pBitFilter	Receives the bitFilter.\n

		@return	Returns an IFXRESULT code.\n
		- @b IFX_OK upon success.\n
		- @b IFX_E_NOT_INITIALIZED if the component has not been initialized.\n
		- @b IFX_E_INVALID_POINTER is returned if pBitFilter is NULL.\n
	*/
	virtual IFXRESULT IFXAPI  GetBitFilter(U32 *pBitFilter) = 0;

	/**
		This method sets the Object filter for this component.

		@param	pObjectFilter	The ObjectFilter pointer. May be NULL.\n

		@return	Returns an IFXRESULT code.\n
		- @b IFX_OK upon success.\n
		- @b IFX_E_NOT_INITIALIZED if the component has not been initialized.\n
	*/
	virtual IFXRESULT IFXAPI  SetObjectFilter(IFXUnknown * pObjectFilter) = 0;

	/**
		This method retrieves the object filter for this component.

		@param	ppObjectFilter	Receives the object filter pointer.\n

		@return	Returns an IFXRESULT code.\n
		- @b IFX_OK upon success.\n
		- @b IFX_E_NOT_INITIALIZED if the component has not been initialized.\n
		- @b IFX_E_INVALID_POINTER is returned if ppObjectFilter is NULL.\n
	*/
	virtual IFXRESULT IFXAPI  GetObjectFilter(IFXUnknown ** ppObjectFilter) = 0;
};


// {76E95C85-A633-11d5-9AE3-00D0B73FB755}
IFXDEFINE_GUID(IID_IFXNotificationInfo,
0x76e95c85, 0xa633, 0x11d5, 0x9a, 0xe3, 0x0, 0xd0, 0xb7, 0x3f, 0xb7, 0x55);


#endif


