#include "LUAAIRegistryTest.h"
#include <string>
#include <fstream>
#include <streambuf>
#include <unistd.h>

class LUAAIRegistryTest: public TestSuite {
protected:
	ai::LUAAIRegistry _registry;
	static std::string _luaCode;
	const ai::CharacterId _id = 1;
	ai::ICharacterPtr _chr = std::make_shared<TestEntity>(_id);
	const ai::ConditionFactoryContext ctxCondition = ai::ConditionFactoryContext("");
	const ai::FilterFactoryContext ctxFilter = ai::FilterFactoryContext("");
	const ai::SteeringFactoryContext ctxSteering = ai::SteeringFactoryContext("");

	static void SetUpTestCase() {
		std::ifstream stream("src/test/luaregistry.lua");
		if (!stream) {
			stream = std::ifstream("luaregistry.lua");
		}
		std::stringstream buffer;
		buffer << stream.rdbuf();
		_luaCode = buffer.str();
	}

	void SetUp() override {
		TestSuite::SetUp();
		ASSERT_FALSE(_luaCode.empty());
		ASSERT_TRUE(_registry.init()) << "Failed to initialize the lua ai registry";
		ASSERT_TRUE(_registry.evaluate(_luaCode)) << "Failed to load lua script:\n" << _luaCode;
	}

	void TearDown() override {
		TestSuite::TearDown();
		_registry.shutdown();
	}

	void testSteering(const char* steeringName, int n = 1) {
		SCOPED_TRACE(steeringName);
		const ai::SteeringPtr& steering = _registry.createSteering(steeringName, ctxSteering);
		ASSERT_TRUE((bool)steering) << "Could not create lua provided steering";
		const ai::AIPtr& ai = std::make_shared<ai::AI>(ai::TreeNodePtr());
		ai->setCharacter(_chr);
		for (int i = 0; i < n; ++i) {
			steering->execute(ai, 1.0f);
		}
		ASSERT_EQ(1, steering.use_count()) << "Someone is still referencing the LUASteering";
	}

	void testFilter(const char* filterName, int n = 1) {
		SCOPED_TRACE(filterName);
		const ai::FilterPtr& filter = _registry.createFilter(filterName, ctxFilter);
		ASSERT_TRUE((bool)filter) << "Could not create lua provided filter";
		const ai::AIPtr& ai = std::make_shared<ai::AI>(ai::TreeNodePtr());
		ai->setCharacter(_chr);
		for (int i = 0; i < n; ++i) {
			filter->filter(ai);
		}
		ASSERT_EQ(1, filter.use_count()) << "Someone is still referencing the LUAFilter";
	}

	void testCondition(const char* conditionName, bool expectedReturnValue, int n = 1) {
		SCOPED_TRACE(conditionName);
		const ai::ConditionPtr& condition = _registry.createCondition(conditionName, ctxCondition);
		ASSERT_TRUE((bool)condition) << "Could not create lua provided condition";
		const ai::AIPtr& ai = std::make_shared<ai::AI>(ai::TreeNodePtr());
		ai->setCharacter(_chr);
		for (int i = 0; i < n; ++i) {
			ASSERT_EQ(expectedReturnValue, condition->evaluate(ai));
		}
		ASSERT_EQ(1, condition.use_count()) << "Someone is still referencing the LUACondition";
	}

	void testNode(const char* nodeName, ai::TreeNodeStatus status, int n = 1) {
		const ai::TreeNodeFactoryContext ctx = ai::TreeNodeFactoryContext("TreeNodeName", "", ai::True::get());
		return testNode(nodeName, status, ctx, n);
	}

	void testNode(const char* nodeName, ai::TreeNodeStatus status, const ai::TreeNodeFactoryContext &ctx, int n = 1) {
		SCOPED_TRACE(nodeName);
		ai::Zone zone("TestNode");
		const ai::TreeNodePtr& node = _registry.createNode(nodeName, ctx);
		ASSERT_TRUE((bool)node) << "Could not create lua provided node '" << nodeName << "'";
		const ai::AIPtr& ai = std::make_shared<ai::AI>(node);
		ai->setCharacter(_chr);
		ASSERT_TRUE(zone.addAI(ai));
		ai->setPause(true);
		zone.update(1l);
		ai->setPause(false);
		for (int i = 0; i < n; ++i) {
			const ai::TreeNodeStatus executionStatus = node->execute(ai, 1L);
			ASSERT_EQ(status, executionStatus) << "Lua script returned an unexpected TreeNodeStatus value";
		}
		ASSERT_TRUE(zone.removeAI(ai));
		ai->setPause(true);
		zone.update(1l);
		ai->setPause(false);
		ai->setBehaviour(ai::TreeNodePtr());
		ASSERT_EQ(1, node.use_count()) << "Someone is still referencing the LUATreeNode";
		ASSERT_EQ(1, ai.use_count()) << "Someone is still referencing the AI instance";
	}
};

std::string LUAAIRegistryTest::_luaCode;

TEST_F(LUAAIRegistryTest, testLuaNode1) {
	testNode("LuaTest", ai::TreeNodeStatus::FINISHED);
}

TEST_F(LUAAIRegistryTest, testLuaNode2) {
	testNode("LuaTest2", ai::TreeNodeStatus::RUNNING);
}

TEST_F(LUAAIRegistryTest, testLuaNode2_100) {
	testNode("LuaTest2", ai::TreeNodeStatus::RUNNING, 100);
}

TEST_F(LUAAIRegistryTest, testCreateInvalidNode) {
	const ai::TreeNodeFactoryContext ctx = ai::TreeNodeFactoryContext("TreeNodeName", "", ai::True::get());
	const ai::TreeNodePtr& node = _registry.createNode("ThisNameDoesNotExist", ctx);
	ASSERT_FALSE((bool)node) << "Created a node for a type that isn't defined";
}

TEST_F(LUAAIRegistryTest, testConditionEvaluationTrue) {
	testCondition("LuaTestTrue", true);
}

TEST_F(LUAAIRegistryTest, testConditionEvaluationTrue_100) {
	testCondition("LuaTestTrue", true, 100);
}

TEST_F(LUAAIRegistryTest, testConditionEvaluationFalse) {
	testCondition("LuaTestFalse", false);
}

TEST_F(LUAAIRegistryTest, testFilterEmpty) {
	testFilter("LuaFilterTest");
}

TEST_F(LUAAIRegistryTest, testFilter_100) {
	testFilter("LuaFilterTest", 100);
}

TEST_F(LUAAIRegistryTest, testSteeringEmpty) {
	testSteering("LuaSteeringTest");
}
