// Copyright 2008-2018 Yolo Technologies, Inc. All Rights Reserved. https://www.comblockengine.com

#include "entity.h"
#include "moveto_point_handler.h"	

namespace KBEngine{	
namespace client
{

//-------------------------------------------------------------------------------------
MoveToPointHandler::MoveToPointHandler(ScriptCallbacks& scriptCallbacks, client::Entity* pEntity, 
											int layer, const Position3D& destPos, 
											 float velocity, float distance, bool faceMovement, 
											bool moveVertically, PyObject* userarg):
ScriptCallbackHandler(scriptCallbacks, NULL),
destPos_(destPos),
velocity_(velocity),
faceMovement_(faceMovement),
moveVertically_(moveVertically),
pyuserarg_(userarg),
distance_(distance),
layer_(layer),
pEntity_(pEntity)
{
}

//-------------------------------------------------------------------------------------
MoveToPointHandler::~MoveToPointHandler()
{
	if(pyuserarg_ != NULL)
	{
		Py_DECREF(pyuserarg_);
	}

	// DEBUG_MSG(fmt::format("MoveToPointHandler::~MoveToPointHandler(): {:p}\n"), (void*)this));
}

//-------------------------------------------------------------------------------------
void MoveToPointHandler::handleTimeout( TimerHandle handle, void * pUser )
{
	update(handle);
}

//-------------------------------------------------------------------------------------
void MoveToPointHandler::onRelease( TimerHandle handle, void * /*pUser*/ )
{
	scriptCallbacks_.releaseCallback(handle);
	delete this;
}

//-------------------------------------------------------------------------------------
bool MoveToPointHandler::requestMoveOver(TimerHandle& handle, const Position3D& oldPos)
{
	pEntity_->onMoveOver(scriptCallbacks_.getIDForHandle(handle), layer_, oldPos, pyuserarg_);
	handle.cancel();
	return true;
}

//-------------------------------------------------------------------------------------
bool MoveToPointHandler::update(TimerHandle& handle)
{
	if(pEntity_ == NULL)
	{
		handle.cancel();
		return false;
	}
	
	Entity* pEntity = pEntity_;
	const Position3D& dstPos = destPos();
	Position3D currpos = pEntity->position();
	Position3D currpos_backup = currpos;
	Direction3D direction = pEntity->direction();

	Vector3 movement = dstPos - currpos;
	if (!moveVertically_) movement.y = 0.f;
	
	bool ret = true;

	if(KBEVec3Length(&movement) < velocity_ + distance_)
	{
		float y = currpos.y;
		currpos = dstPos;

		if(distance_ > 0.0f)
		{
			// ??????????
			KBEVec3Normalize(&movement, &movement); 
			movement *= distance_;
			currpos -= movement;
		}

		if (!moveVertically_)
			currpos.y = y;

		ret = false;
	}
	else
	{
		// ??????????
		KBEVec3Normalize(&movement, &movement); 

		// ????????
		movement *= velocity_;
		currpos += movement;
	}
	
	// ????????????????
	if (faceMovement_ && (movement.x != 0.f || movement.z != 0.f))
		direction.yaw(movement.yaw());
	
	// ????entity??????????????
	pEntity_->clientPos(currpos);
	pEntity_->clientDir(direction);

	// ??navigate????????????????????
	pEntity_->isOnGround(false);

	// ????????
	pEntity->onMove(scriptCallbacks_.getIDForHandle(handle), layer_, currpos_backup, pyuserarg_);

	// ????????????????????true
	if(!ret)
	{
		return !requestMoveOver(handle, currpos_backup);
	}

	return true;
}

//-------------------------------------------------------------------------------------
}
}
