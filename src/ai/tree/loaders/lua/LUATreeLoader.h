#pragma once

#include "tree/loaders/ITreeLoader.h"
#include "tree/TreeNodeImpl.h"
#include "AIRegistry.h"
#include "conditions/ConditionParser.h"
#include "tree/TreeNodeParser.h"
#include "LUA.h"

namespace ai {

/**
 * @brief Implementation of @c ITreeLoader that gets its data from a lua script
 */
class LUATreeLoader: public ai::ITreeLoader {
private:
	class LUATree;

	class LUACondition {
	private:
		const ConditionPtr& _condition;
	public:
		LUACondition(const ConditionPtr& condition) :
				_condition(condition) {
		}

		inline const ConditionPtr& getCondition() const {
			return _condition;
		}
	};

	class LUANode {
	private:
		TreeNodePtr _node;
		LUACondition *_condition;
		std::vector<LUANode*> _children;
		LUATree *_tree;
		const IAIFactory& _aiFactory;
	public:
		LUANode(const TreeNodePtr& node, LUATree *tree, const IAIFactory& aiFactory) :
				_node(node), _condition(nullptr), _tree(tree), _aiFactory(aiFactory) {
		}

		~LUANode() {
			delete _condition;
		}

		inline const IAIFactory& getAIFactory() const{
			return _aiFactory;
		}

		inline TreeNodePtr& getTreeNode() {
			return _node;
		}

		inline const TreeNodePtr& getTreeNode() const {
			return _node;
		}

		inline void setCondition(LUACondition *condition) {
			_condition = condition;
			_node->setCondition(condition->getCondition());
		}

		inline const std::vector<LUANode*>& getChildren() const {
			return _children;
		}

		inline LUACondition* getCondition() const {
			return _condition;
		}

		LUANode* addChild (const std::string& nodeType, const TreeNodeFactoryContext& ctx) {
			TreeNodeParser parser(_aiFactory, nodeType);
			const TreeNodePtr& child = parser.getTreeNode(ctx.name);
			if (!child) {
				return nullptr;
			}
			LUANode *node = new LUANode(child, _tree, _aiFactory);
			_children.push_back(node);
			_node->addChild(child);
			return node;
		}
	};


	class LUATree {
	private:
		std::string _name;
		LUATreeLoader* _ctx;
		LUANode* _root;
	public:
		LUATree(const std::string& name, LUATreeLoader* ctx) :
				_name(name), _ctx(ctx), _root(nullptr) {
		}

		inline const IAIFactory& getAIFactory() const{
			return _ctx->getAIFactory();
		}

		inline bool setRoot(LUANode* root) {
			if (_ctx->addTree(_name, root->getTreeNode())) {
				_root = root;
				return true;
			}

			return false;
		}

		inline const std::string& getName() const {
			return _name;
		}

		inline LUANode* getRoot() const {
			return _root;
		}
	};

	static LUATreeLoader* luaGetContext(lua_State * l) {
		return LUA::getGlobalData<LUATreeLoader>(l, "Loader");
	}

	static LUATree* luaGetTreeContext(lua_State * l, int n) {
		return LUA::getUserData<LUATree>(l, n, "Tree");
	}

	static LUANode* luaGetNodeContext(lua_State * l, int n) {
		return LUA::getUserData<LUANode>(l, n, "Node");
	}

	#if 0
	static LUACondition* luaGetConditionContext(lua_State * l, int n) {
		return LUA::getUserData<LUACondition>(l, n, "Condition");
	}
	#endif

	static int luaMain_CreateTree(lua_State * l) {
		LUATreeLoader *ctx = luaGetContext(l);
		const std::string name = luaL_checkstring(l, 1);
		LUATree** udata = LUA::newUserdata<LUATree>(l, "Tree");
		*udata = new LUATree(name, ctx);
		return 1;
	}

	static int luaTree_GC(lua_State * l) {
		LUATree *tree = luaGetTreeContext(l, 1);
		delete tree;
		return 0;
	}

	static int luaTree_ToString(lua_State * l) {
		const LUATree *tree = luaGetTreeContext(l, 1);
		lua_pushfstring(l, "tree: %s [%s]", tree->getName().c_str(), tree->getRoot() ? "root" : "no root");
		return 1;
	}

	static int luaTree_GetName(lua_State * l) {
		const LUATree *tree = luaGetTreeContext(l, 1);
		lua_pushstring(l, tree->getName().c_str());
		return 1;
	}

	static int luaNode_GC(lua_State * l) {
		LUANode *node = luaGetNodeContext(l, 1);
		delete node;
		return 0;
	}

	static int luaNode_ToString(lua_State * l) {
		const LUANode *node = luaGetNodeContext(l, 1);
		const LUACondition* condition = node->getCondition();
		lua_pushfstring(l, "node: %d children [condition: %s]", (int)node->getChildren().size(),
				condition ? condition->getCondition()->getName().c_str() : "no condition");
		return 1;
	}

	static int luaNode_GetName(lua_State * l) {
		const LUANode *node = luaGetNodeContext(l, 1);
		lua_pushstring(l, node->getTreeNode()->getName().c_str());
		return 1;
	}

	static int luaTree_CreateRoot(lua_State * l) {
		LUATree *ctx = luaGetTreeContext(l, 1);
		const std::string id = luaL_checkstring(l, 2);
		const std::string name = luaL_checkstring(l, 3);

		TreeNodeParser parser(ctx->getAIFactory(), id);
		const TreeNodePtr& node = parser.getTreeNode(name);
		if (!node) {
			LUA::returnError(l, "Could not create a node for " + id);
			return 0;
		}

		LUANode** udata = LUA::newUserdata<LUANode>(l, "Node");
		*udata = new LUANode(node, ctx, ctx->getAIFactory());
		if (!ctx->setRoot(*udata)) {
			LUATreeLoader *loader = luaGetContext(l);
			LUA::returnError(l, loader->getError());
			return 0;
		}
		return 1;
	}

	static int luaNode_AddNode(lua_State * l) {
		LUANode *node = luaGetNodeContext(l, 1);
		const std::string id = luaL_checkstring(l, 2);
		const std::string name = luaL_checkstring(l, 3);
		LUANode** udata = LUA::newUserdata<LUANode>(l, "Node");

		TreeNodeFactoryContext factoryCtx(name, "", True::get());
		*udata = node->addChild(id, factoryCtx);
		if (*udata == nullptr) {
			LUA::returnError(l, "Could not create a node for " + id);
		}
		return 1;
	}

	static int luaNode_SetCondition(lua_State * l) {
		LUATreeLoader *ctx = luaGetContext(l);
		LUANode *node = luaGetNodeContext(l, 1);
		const std::string conditionExpression = luaL_checkstring(l, 2);
		LUACondition** udata = LUA::newUserdata<LUACondition>(l, "Condition");

		ConditionParser parser(ctx->getAIFactory(), conditionExpression);
		const ConditionPtr& condition = parser.getCondition();
		if (!condition) {
			LUA::returnError(l, "Could not create a condition for " + conditionExpression + ": " + parser.getError());
			return 0;
		}

		*udata = new LUACondition(condition);
		node->setCondition(*udata);
		return 1;
	}

public:
	LUATreeLoader(const IAIFactory& aiFactory) :
			ITreeLoader(aiFactory) {
	}
	/**
	 * @brief this will initialize the loader once with all the defined behaviours from the given lua string.
	 */
	bool init(const std::string& luaString) {
		setError("");
		_treeMap.clear();

		LUA lua;
		luaL_Reg createTree = { "createTree", luaMain_CreateTree };
		luaL_Reg eof = { nullptr, nullptr };
		luaL_Reg funcs[] = { createTree, eof };

		LUAType tree = lua.registerType("Tree");
		tree.addFunction("createRoot", luaTree_CreateRoot);
		tree.addFunction("getName", luaTree_GetName);
		tree.addFunction("__gc", luaTree_GC);
		tree.addFunction("__tostring", luaTree_ToString);

		LUAType node = lua.registerType("Node");
		node.addFunction("addNode", luaNode_AddNode);
		node.addFunction("getName", luaNode_GetName);
		node.addFunction("setCondition", luaNode_SetCondition);
		node.addFunction("__gc", luaNode_GC);
		node.addFunction("__tostring", luaNode_ToString);

		lua.reg("AI", funcs);

		if (!lua.load(luaString)) {
			setError(lua.getError());
			return false;
		}

		// loads all the trees
		lua.newGlobalData<LUATreeLoader>("Loader", this);
		if (!lua.execute("init")) {
			setError(lua.getError());
			return false;
		}

		if (_treeMap.empty()) {
			setError("No behaviour trees specified");
			return false;
		}
		return true;
	}
};

}
