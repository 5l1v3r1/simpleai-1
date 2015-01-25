#include "tree/PrioritySelector.h"
#include "AI.h"

namespace ai {

TreeNodeStatus PrioritySelector::execute(AI& entity, long deltaMillis) {
	// TODO: don't reset - but continue until the running node finished
	setSelectorState(entity, NOTHING_SELECTED);
	if (Selector::execute(entity, deltaMillis) == CANNOTEXECUTE)
		return CANNOTEXECUTE;

	const std::size_t size = _children.size();
	TreeNodeStatus overallResult = FINISHED;
	std::size_t i;
	for (i = 0; i < size; ++i) {
		const TreeNodePtr& child = _children[i];
		const TreeNodeStatus result = child->execute(entity, deltaMillis);
		if (result == RUNNING) {
			setSelectorState(entity, static_cast<int>(i));
		} else if (result == CANNOTEXECUTE || result == FAILED) {
			child->resetState(entity);
			continue;
		}
		child->resetState(entity);
		overallResult = result;
		break;
	}
	for (++i; i < size; ++i) {
		_children[i]->resetState(entity);
	}
	return state(entity, overallResult);
}

NODE_FACTORY_IMPL(PrioritySelector)

}
