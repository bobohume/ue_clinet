#include "Entity.h"
#include "message/Packet.h"
#include "message/game.pb.h"

using namespace message;
Entity::Entity()
{
	REGISTER_PACKET(new W_C_ENTITY(), std::bind(&Entity::_W_C_ENTITY, this, std::placeholders::_1));
}

bool Entity::_W_C_ENTITY(::google::protobuf::Message* _packet) {
	auto packet = (W_C_ENTITY*)(_packet);
	if (!packet) {
		return false;
	}

	for (auto i = 0; i < packet->entityinfo_size(); i++) {
		const W_C_ENTITY_Entity pEntityInfo = packet->entityinfo(i);
		if (pEntityInfo.has_data()) {
			
		}

		if (pEntityInfo.has_move()) {
		
		}
	}


	return true;
}