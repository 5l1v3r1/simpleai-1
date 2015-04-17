#include "GroupTest.h"

class GroupMgrTest: public TestSuite {
};

TEST_F(GroupMgrTest, testMassGroupAveragePosition) {
	const ai::GroupId groupId = 1;
	const ai::Vector3f pos1(3.0f, 3.0f, 0.0f);
	const ai::Vector3f pos2(300.0f, 300.0f, 0.0f);
	for (int i = 1; i <= 2; ++i) {
		const ai::CharacterId id = i;
		ai::AIPtr e(new ai::AI(ai::TreeNodePtr()));
		ai::ICharacterPtr chr(new ai::ICharacter(id));
		e->setCharacter(chr);
		chr->setPosition(pos1);
		_groupManager.add(groupId, e);
	}
	for (int i = 3; i <= 4; ++i) {
		const ai::CharacterId id = i;
		ai::AIPtr e(new ai::AI(ai::TreeNodePtr()));
		ai::ICharacterPtr chr(new ai::ICharacter(id));
		e->setCharacter(chr);
		chr->setPosition(pos2);
		_groupManager.add(groupId, e);
	}
	_groupManager.update(0);
	const ai::Vector3f& avg = _groupManager.getPosition(groupId);
	const ai::Vector3f pos = pos1 + pos2;

	ASSERT_EQ(0.5f * pos, avg);
}

class GroupTest: public TestSuite {
private:
public:
	typedef std::vector<ai::AIPtr> TestEntities;
	inline void addMass(int max, ai::GroupId groupId, TestEntities& ais, ai::GroupMgr& mgr) const {
		for (int i = 1; i <= max; ++i) {
			const ai::CharacterId id = i;
			ai::AIPtr e(new ai::AI(ai::TreeNodePtr()));
			ai::ICharacterPtr chr(new ai::ICharacter(id));
			e->setCharacter(chr);
			ais.push_back(e);
			mgr.add(groupId, e);
		}
		ASSERT_EQ(max, mgr.getGroupSize(groupId));
	}

	inline void remove(ai::GroupId groupId, TestEntities& ais, ai::GroupMgr& mgr) const {
		for (auto i = ais.begin(); i != ais.end(); ++i) {
			mgr.remove(groupId, *i);
		}
		const int nEmpty = mgr.getGroupSize(groupId);
		ASSERT_EQ(0, nEmpty) << "Unexpected group size for " << groupId;
	}

	void doMassTest(int max) {
		TestEntities ais(max);
		const ai::GroupId groupId = 1;
		ai::GroupMgr mgr;

		addMass(max, groupId, ais, mgr);
		remove(groupId, ais, mgr);
	}
};

TEST_F(GroupTest, testGroupAddRemove) {
	const ai::GroupId id = 1;
	ai::GroupMgr groupMgr;
	ai::AIPtr entity1(new ai::AI(ai::TreeNodePtr()));
	ai::ICharacterPtr chr(new ai::ICharacter(id));
	entity1->setCharacter(chr);
	ASSERT_TRUE(groupMgr.add(id, entity1));
	ASSERT_FALSE(groupMgr.remove(0, entity1));
	ASSERT_TRUE(groupMgr.remove(id, entity1));
	ASSERT_FALSE(groupMgr.remove(id, entity1));
}

TEST_F(GroupTest, testGroupIsInAny) {
	const ai::GroupId id = 1;
	ai::GroupMgr groupMgr;
	ai::AIPtr entity1(new ai::AI(ai::TreeNodePtr()));
	ai::ICharacterPtr chr(new ai::ICharacter(id));
	entity1->setCharacter(chr);
	ASSERT_TRUE(groupMgr.add(id, entity1));
	ASSERT_TRUE(groupMgr.isInAnyGroup(entity1));
	ASSERT_TRUE(groupMgr.remove(id, entity1));
	ASSERT_FALSE(groupMgr.isInAnyGroup(entity1));
}

TEST_F(GroupTest, testGroupSize) {
	const ai::GroupId id = 1;
	ai::GroupMgr groupMgr;
	ai::AIPtr entity1(new ai::AI(ai::TreeNodePtr()));
	ai::ICharacterPtr chr(new ai::ICharacter(id));
	entity1->setCharacter(chr);
	ASSERT_TRUE(groupMgr.add(id, entity1));
	ai::AIPtr entity2(new ai::AI(ai::TreeNodePtr()));
	entity2->setCharacter(ai::ICharacterPtr(new ai::ICharacter(2)));
	ASSERT_TRUE(groupMgr.add(id, entity2));
	ASSERT_EQ(2, groupMgr.getGroupSize(id));
}

TEST_F(GroupTest, testGroupLeader) {
	const ai::GroupId id = 1;
	ai::GroupMgr groupMgr;
	ai::AIPtr entity1(new ai::AI(ai::TreeNodePtr()));
	ai::ICharacterPtr chr(new ai::ICharacter(id));
	entity1->setCharacter(chr);
	ASSERT_TRUE(groupMgr.add(id, entity1));
	ai::AIPtr entity2(new ai::AI(ai::TreeNodePtr()));
	entity2->setCharacter(ai::ICharacterPtr(new ai::ICharacter(2)));
	ASSERT_TRUE(groupMgr.add(id, entity2));
	ai::AIPtr entity3(new ai::AI(ai::TreeNodePtr()));
	entity3->setCharacter(ai::ICharacterPtr(new ai::ICharacter(3)));
	ASSERT_TRUE(groupMgr.add(id, entity3));
	ASSERT_EQ(3, groupMgr.getGroupSize(id));
	ASSERT_TRUE(groupMgr.isGroupLeader(id, entity1));
	ASSERT_FALSE(groupMgr.isGroupLeader(id, entity2));
	ASSERT_FALSE(groupMgr.isGroupLeader(id, entity3));
	ASSERT_TRUE(groupMgr.remove(id, entity1));
	ASSERT_FALSE(groupMgr.isInGroup(id, entity1));
	ASSERT_FALSE(groupMgr.isGroupLeader(id, entity1));
	ASSERT_TRUE(groupMgr.isGroupLeader(id, entity2) || groupMgr.isGroupLeader(id, entity3));
}

TEST_F(GroupTest, testGroupAveragePosition) {
	const ai::GroupId id = 1;
	ai::Vector3f avg;
	ai::GroupMgr groupMgr;
	ai::AIPtr entity1(new ai::AI(ai::TreeNodePtr()));
	ai::ICharacterPtr chr(new ai::ICharacter(id));
	entity1->setCharacter(chr);
	chr->setPosition(ai::Vector3f(1.0f, 1.0f, 0.0f));
	ASSERT_TRUE(groupMgr.add(id, entity1));
	groupMgr.update(0);
	avg = groupMgr.getPosition(id);
	ASSERT_EQ(ai::Vector3f(1.0f, 1.0f, 0.0f), avg);
	ai::AIPtr entity2(new ai::AI(ai::TreeNodePtr()));
	entity2->setCharacter(ai::ICharacterPtr(new ai::ICharacter(2)));
	entity2->getCharacter()->setPosition(ai::Vector3f(3.0f, 3.0f, 0.0f));
	ASSERT_TRUE(groupMgr.add(id, entity2));
	groupMgr.update(0);
	avg = groupMgr.getPosition(id);
	ASSERT_EQ(ai::Vector3f(2.0f, 2.0f, 0.0f), avg);
}

TEST_F(GroupTest, testGroupMass1000) {
	doMassTest(1000);
}

TEST_F(GroupTest, testGroupMass10000) {
	doMassTest(10000);
}

class GroupMassTest: public GroupTest {
protected:
	GroupTest::TestEntities _ais;
	const int _maxUsers = 100;
	const int _maxGroups = 100;
public:
	GroupMassTest() : GroupTest() {
		_ais.clear();
		_ais.reserve(_maxUsers * _maxGroups);
		for (int i = 0; i < _maxGroups;++i) {
			const ai::GroupId groupId = i;
			addMass(_maxUsers, groupId, _ais, _groupManager);
		}
	}
};

TEST_F(GroupMassTest, testIsInAnyGroupMass100x100) {
	const ai::AIPtr& e = _ais.back();
	ASSERT_TRUE(_groupManager.isInAnyGroup(e));
}

TEST_F(GroupTest, testGroupRemove) {
	const ai::GroupId id = 1;
	ai::GroupMgr groupMgr;
	ai::AIPtr entity1(new ai::AI(ai::TreeNodePtr()));
	entity1->setCharacter(ai::ICharacterPtr(new ai::ICharacter(1)));
	ASSERT_TRUE(groupMgr.add(id, entity1));
	ai::AIPtr entity2(new ai::AI(ai::TreeNodePtr()));
	entity2->setCharacter(ai::ICharacterPtr(new ai::ICharacter(2)));
	ASSERT_TRUE(groupMgr.add(id, entity2));
	ai::AIPtr entity3(new ai::AI(ai::TreeNodePtr()));
	entity3->setCharacter(ai::ICharacterPtr(new ai::ICharacter(3)));
	ASSERT_TRUE(groupMgr.add(id, entity3));
	ASSERT_EQ(3, groupMgr.getGroupSize(id));
	ASSERT_TRUE(groupMgr.remove(id, entity1));
	ASSERT_TRUE(groupMgr.remove(id, entity2));
	ASSERT_TRUE(groupMgr.remove(id, entity3));
	ASSERT_EQ(0, groupMgr.getGroupSize(id));
}
