#pragma once

#include "IProtocolMessage.h"
#include "AIStubTypes.h"

namespace ai {

/**
 * @brief Message for the remote debugging interface
 *
 * Updates a node to some other type, name or condition
 */
class AIUpdateNodeMessage: public IProtocolMessage {
private:
	int32_t _nodeId;
	std::string _name;
	std::string _type;
	std::string _condition;

public:
	AIUpdateNodeMessage(int32_t nodeId, const std::string& name, const std::string& type, const std::string& condition) :
			IProtocolMessage(PROTO_UPDATENODE), _nodeId(nodeId), _name(name), _type(type), _condition(condition) {
	}

	AIUpdateNodeMessage(streamContainer& in) :
			IProtocolMessage(PROTO_UPDATENODE) {
		_nodeId = readInt(in);
		_name = readString(in);
		_type = readString(in);
		_condition = readString(in);
	}

	void serialize(streamContainer& out) const override {
		addByte(out, _id);
		addInt(out, _nodeId);
		addString(out, _name);
		addString(out, _type);
		addString(out, _condition);
	}

	inline const std::string& getName() const {
		return _name;
	}

	inline const std::string& getType() const {
		return _type;
	}

	inline const std::string& getCondition() const {
		return _condition;
	}

	inline uint32_t getNodeId() const {
		return _nodeId;
	}
};

}
