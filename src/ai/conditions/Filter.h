#pragma once

#include <conditions/ICondition.h>
#include <filter/IFilter.h>

#define FILTER_NAME "Filter"

namespace ai {

/**
 * @brief The filter condition executes some selection filters (@c IFilter) and evaluates to @c true if
 * the resulting set of all filter executions is non-empty. Use @c AI::getFilteredEntities to access the
 * result set and work with it in a TreeNode that is executed when this condition evaluated to true
 */
class Filter: public ICondition {
protected:
	Filters _filters;

	void getConditionNameWithValue(std::stringstream& s, const AI& entity) override;

public:
	class Factory: public IConditionFactory {
	public:
		ConditionPtr create (const ConditionFactoryContext *ctx) const;
	};
	static Factory FACTORY;

	Filter (const Filters& filters);

	/**
	 * @brief Executes the attached filters and wiped the last filter results for the given @c AI entity
	 *
	 * @return @c true if the attached filters lead to a non-empty set, @c false otherwise
	 */
	bool evaluate(const AI& entity) override;
};

}
