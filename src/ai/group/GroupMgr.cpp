#include "GroupMgr.h"
#include "ICharacter.h"
#include <numeric>

namespace ai {

struct AveragePositionFunctor {
	glm::vec3 operator()(const glm::vec3& result, const AIPtr& ai) {
		return ai->getCharacter()->getPosition() + result;
	}
};

GroupMgr::GroupMgr() {
}

GroupMgr::~GroupMgr() {
}

void GroupMgr::update(int64_t) {
	ScopedReadLock scopedLock(_lock);
	for (auto i = _groups.begin(); i != _groups.end(); ++i) {
		Group& group = i->second;
		glm::vec3 averagePosition;
		{
			ScopedReadLock lock(_groupLock);
			averagePosition = std::accumulate(group.members.begin(), group.members.end(), glm::vec3(), AveragePositionFunctor());
			averagePosition *= 1.0f / (float) group.members.size();
		}
		ScopedWriteLock lock(_groupLock);
		group.position = averagePosition;
	}
}

bool GroupMgr::add(GroupId id, const AIPtr& ai) {
	ScopedWriteLock scopedLock(_lock);
	GroupsIter i = _groups.find(id);
	if (i == _groups.end()) {
		Group group;
		group.leader = ai;
		i = _groups.insert(std::pair<GroupId, Group>(id, group)).first;
	}

	Group& group = i->second;
	ScopedWriteLock lock(_groupLock);
	std::pair<GroupMembersSetIter, bool> ret = group.members.insert(ai);
	if (ret.second) {
		_groupMembers.insert(GroupMembers::value_type(ai, id));
		return true;
	}
	return false;
}

bool GroupMgr::remove(GroupId id, const AIPtr& ai) {
	ScopedWriteLock scopedLock(_lock);
	const GroupsIter& i = _groups.find(id);
	if (i == _groups.end())
		return false;
	Group& group = i->second;
	GroupMembersSetIter si;
	{
		ScopedReadLock lock(_groupLock);
		si = group.members.find(ai);
		if (si == group.members.end())
			return false;
	}
	{
		ScopedWriteLock lock(_groupLock);
		group.members.erase(si);
		if (group.members.empty())
			_groups.erase(i);
		else if (group.leader == ai)
			group.leader = *group.members.begin();
	}

	auto range = _groupMembers.equal_range(ai);
	for (auto it = range.first; it != range.second; ++it) {
		if (it->second == id) {
			_groupMembers.erase(it);
			break;
		}
	}
	return true;
}

bool GroupMgr::removeFromAllGroups(const AIPtr& ai) {
	std::list<GroupId> groups;
	{
		ScopedReadLock scopedLock(_lock);
		auto range = _groupMembers.equal_range(ai);
		for (auto it = range.first; it != range.second; ++it) {
			groups.push_back(it->second);
		}
	}
	for (GroupId groupId : groups) {
		remove(groupId, ai);
	}
	return true;
}

AIPtr GroupMgr::getLeader(GroupId id) const {
	ScopedReadLock scopedLock(_lock);
	const GroupsConstIter& i = _groups.find(id);
	if (i == _groups.end())
		return AIPtr();

	ScopedReadLock lock(_groupLock);
	return i->second.leader;
}

glm::vec3 GroupMgr::getPosition(GroupId id) const {
	ScopedReadLock scopedLock(_lock);
	const GroupsConstIter& i = _groups.find(id);
	if (i == _groups.end())
		return INFINITE;

	ScopedReadLock lock(_groupLock);
	return i->second.position;
}

bool GroupMgr::isGroupLeader(GroupId id, const AIPtr& ai) const {
	ScopedReadLock scopedLock(_lock);
	const GroupsConstIter& i = _groups.find(id);
	if (i == _groups.end()) {
		return 0;
	}
	ScopedReadLock lock(_groupLock);
	return i->second.leader == ai;
}

int GroupMgr::getGroupSize(GroupId id) const {
	ScopedReadLock scopedLock(_lock);
	const GroupsConstIter& i = _groups.find(id);
	if (i == _groups.end()) {
		return 0;
	}
	ScopedReadLock lock(_groupLock);
	return static_cast<int>(std::distance(i->second.members.begin(), i->second.members.end()));
}

bool GroupMgr::isInAnyGroup(const AIPtr& ai) const {
	ScopedReadLock scopedLock(_lock);
	return _groupMembers.find(ai) != _groupMembers.end();
}

bool GroupMgr::isInGroup(GroupId id, const AIPtr& ai) const {
	ScopedReadLock scopedLock(_lock);
	auto range = _groupMembers.equal_range(ai);
	for (auto it = range.first; it != range.second; ++it) {
		if (it->second == id)
			return true;
	}
	return false;
}

}
