#pragma once

#include "tree/TreeNode.h"

namespace ai {

#define TASK_CLASS_CTOR(TaskName) \
	TaskName(const std::string& name, const std::string& parameters, const ConditionPtr& condition) : \
			ITask(name, parameters, condition)
#define TASK_CLASS_DTOR(TaskName) virtual ~TaskName()

#define TASK_CLASS(TaskName) \
	TASK_CLASS_CTOR(TaskName) {}\
	TASK_CLASS_DTOR(TaskName) {}

/**
 * @brief A node for your real actions in the behaviour tree
 * @note This node doesn't support children
 */
class ITask: public TreeNode {
protected:
	TreeNodeStatus execute(const AIPtr& entity, long deltaMillis);
public:
	ITask(const std::string& name, const std::string& parameters, const ConditionPtr& condition);
	virtual ~ITask();

	/**
	 * @note The returned @c TreeNodeStatus is automatically recorded. This method is only
	 * called when the attached @c ICondition evaluated to @c true
	 */
	virtual TreeNodeStatus doAction(const AIPtr& entity, long deltaMillis) = 0;

	bool addChild(const TreeNodePtr& child) override;
};

}
