#include "conditions/Filter.h"
#include "AIRegistry.h"

namespace ai {

Filter::Filter (const Filters& filters) :
		ICondition(FILTER_NAME, ""), _filters(filters) {
}

void Filter::getConditionNameWithValue(std::stringstream& s, const AIPtr& entity) {
	bool first = true;
	s << "(";
	auto copy = entity->_filteredEntities;
	for (const FilterPtr& filter : _filters) {
		if (!first)
			s << ",";
		s << filter->getName() << "{" << filter->getParameters() << "}[";
		entity->_filteredEntities.clear();
		filter->filter(entity);
		bool firstChr = true;
		int cnt = 0;
		for (CharacterId id : entity->_filteredEntities) {
			if (!firstChr)
				s << ",";
			s << id;
			firstChr = false;
			++cnt;
			if (cnt > 15) {
				s << ",...";
				break;
			}
		}
		s << "]";
		first = false;
	}
	entity->_filteredEntities = copy;
	s << ")";
}

bool Filter::evaluate(const AIPtr& entity) {
	entity->_filteredEntities.clear();
	for (const FilterPtr filter : _filters) {
		filter->filter(entity);
	}
	return !entity->_filteredEntities.empty();
}

ConditionPtr Filter::Factory::create(const ConditionFactoryContext *ctx) const {
	Filter* c = new Filter(ctx->filters);
	return ConditionPtr(c);
}

Filter::Factory Filter::FACTORY;

}
