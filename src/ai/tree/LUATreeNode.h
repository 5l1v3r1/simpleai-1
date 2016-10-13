/**
 * @file
 */
#pragma once

#include "tree/TreeNode.h"
#include "common/LUA.h"

namespace ai {

/**
 * @see @ai{LUAAIRegistry}
 */
class LUATreeNode : public TreeNode {
public:
	static inline const char *luaAIMetaName() {
		return "entity";
	}
protected:
	lua_State* _s;

	TreeNodeStatus runLUA(const AIPtr& entity, int64_t deltaMillis) {
		// get userdata of the behaviour tree node
		const std::string name = "__meta_node_" + _type;
		lua_getfield(_s, LUA_REGISTRYINDEX, name.c_str());
#ifdef AI_LUA_SANTITY
		if (lua_isnil(_s, -1)) {
			ai_log_error("LUA node: could not find lua userdata for %s", name.c_str());
			return TreeNodeStatus::EXCEPTION;
		}
#endif
		// get metatable
		lua_getmetatable(_s, -1);
#ifdef AI_LUA_SANTITY
		if (!lua_istable(_s, -1)) {
			ai_log_error("LUA node: userdata for %s doesn't have a metatable assigned", name.c_str());
			return TreeNodeStatus::EXCEPTION;
		}
#endif
		// get execute() method
		lua_getfield(_s, -1, "execute");
		if (!lua_isfunction(_s, -1)) {
			ai_log_error("LUA node: metatable for %s doesn't have the execute() function assigned", name.c_str());
			return TreeNodeStatus::EXCEPTION;
		}

		// push self onto the stack
		lua_getfield(_s, LUA_REGISTRYINDEX, name.c_str());

		// first parameter is ai
		AI ** udata = (AI **) lua_newuserdata(_s, sizeof(AI *));
		luaL_getmetatable(_s, luaAIMetaName());
#ifdef AI_LUA_SANTITY
		if (!lua_istable(_s, -1)) {
			ai_log_error("LUA node: metatable for %s doesn't exist", luaAIMetaName());
			return TreeNodeStatus::EXCEPTION;
		}
#endif
		lua_setmetatable(_s, -2);
		*udata = entity.get();

		// second parameter is dt
		lua_pushinteger(_s, deltaMillis);

#ifdef AI_LUA_SANTITY
		if (!lua_isfunction(_s, -4)) {
			ai_log_error("LUA node: expected to find a function on stack -4");
			return TreeNodeStatus::EXCEPTION;
		}
		if (!lua_isuserdata(_s, -3)) {
			ai_log_error("LUA node: expected to find the userdata on -3");
			return TreeNodeStatus::EXCEPTION;
		}
		if (!lua_isuserdata(_s, -2)) {
			ai_log_error("LUA node: second parameter should be the ai");
			return TreeNodeStatus::EXCEPTION;
		}
		if (!lua_isinteger(_s, -1)) {
			ai_log_error("LUA node: first parameter should be the delta millis");
			return TreeNodeStatus::EXCEPTION;
		}
#endif
		const int error = lua_pcall(_s, 3, 1, 0);
		if (error) {
			ai_log_error("LUA node script: %s", lua_isstring(_s, -1) ? lua_tostring(_s, -1) : "Unknown Error");
			// reset stack
			lua_pop(_s, lua_gettop(_s));
			return TreeNodeStatus::EXCEPTION;
		}
		const int state = luaL_checkinteger(_s, -1);
		if (state < 0 || state >= (int)TreeNodeStatus::MAX_TREENODESTATUS) {
			ai_log_error("LUA node: illegal tree node status returned: %i", state);
		}

		// reset stack
		lua_pop(_s, lua_gettop(_s));
		return (TreeNodeStatus)state;
	}

public:
	class LUATreeNodeFactory : public ITreeNodeFactory {
	private:
		lua_State* _s;
		std::string _type;
	public:
		LUATreeNodeFactory(lua_State* s, const std::string& type) :
				_s(s), _type(type) {
		}

		inline const std::string& type() const {
			return _type;
		}

		TreeNodePtr create(const TreeNodeFactoryContext* ctx) const override {
			return std::make_shared<LUATreeNode>(ctx->name, ctx->parameters, ctx->condition, _s, _type);
		}
	};

	LUATreeNode(const std::string& name, const std::string& parameters, const ConditionPtr& condition, lua_State* s, const std::string& type) :
			TreeNode(name, parameters, condition), _s(s) {
		_type = type;
	}

	~LUATreeNode() {
	}

	TreeNodeStatus execute(const AIPtr& entity, int64_t deltaMillis) override {
		if (TreeNode::execute(entity, deltaMillis) == CANNOTEXECUTE) {
			return CANNOTEXECUTE;
		}

#if AI_EXCEPTIONS
		try {
#endif
			return state(entity, runLUA(entity, deltaMillis));
#if AI_EXCEPTIONS
		} catch (...) {
			ai_log_error("Exception while running lua tree node")
			return state(entity, EXCEPTION);
		}
#endif
	}
};

}
