#include <random>
#include <chrono>
#include <vector>
#include <set>
#include <map>
#include <cstring>
#include <exception>

#include "MyStrategy.hpp"
MyStrategy::MyStrategy() {}


class HfsmNode;

enum class NodeType
{
	HfsmNode,
	MinerNode,
	Repairer,
	BuilderNode,
};

namespace HfsmData
{
	std::map<int, std::shared_ptr<HfsmNode>>hfsmStates;
	std::map<int, Entity>viewById;
	PlayerView playerView;
	Player player;
	EntityProperties BUILDER_UNIT;
	EntityProperties HOUSE;
	EntityProperties MELEE_BASE;
	EntityProperties MELEE_UNIT;
	EntityProperties RANGED_BASE;
	EntityProperties RANGED_UNIT;
	EntityProperties RESOURCE;
	EntityProperties WALL;
	EntityProperties TURRET;
	bool vis[305][305], visM[305][305];
	int dis(Entity x, Entity y)
	{
		return abs(x.position.x - y.position.x) + abs(x.position.y - y.position.y);
	}
	int dis(Vec2Int x, Vec2Int y)
	{
		return abs(x.x - y.x) + abs(x.y - y.y);
	}
	bool canFull(Vec2Int pos, int size)
	{
		for (int i = pos.x; i < pos.x + size; i++)
		{
			for (int j = pos.y; j < pos.y + size; j++)
			{
				if (vis[i][j])return 0;
				if (i < 0 || i >= playerView.mapSize)return 0;
				if (j < 0 || j >= playerView.mapSize)return 0;
			}
		}
		return 1;
	}
	void makeFull(Vec2Int pos, int size)
	{
		for (int i = pos.x; i < pos.x + size; i++)for (int j = pos.y; j < pos.y + size; j++)vis[i][j] = 1;
	}
	bool canFullM(Vec2Int pos, int size)
	{
		for (int i = pos.x; i < pos.x + size; i++)
		{
			for (int j = pos.y; j < pos.y + size; j++)
			{
				if (visM[i][j])return 0;
				if (i<0||i >= playerView.mapSize)return 0;
				if (j<0||j >= playerView.mapSize)return 0;
			}
		}
		return 1;
	}
	void makeFullM(Vec2Int pos, int size)
	{
		for (int i = pos.x; i < pos.x + size; i++)for (int j = pos.y; j < pos.y + size; j++)visM[i][j] = 1;
	}
	Vec2Int finRound(Vec2Int pos, int size, Vec2Int self)
	{
		std::vector<Vec2Int>v;
		for (int p = 0; p < size; p++)
		{
			if (HfsmData::canFull({ pos.x + size ,pos.y + p }, 1))v.push_back(Vec2Int(pos.x + size, pos.y + p));
			if (HfsmData::canFull({ pos.x - 1 ,pos.y + p }, 1))v.push_back(Vec2Int(pos.x - 1, pos.y + p));
			if (HfsmData::canFull({ pos.x + p ,pos.y + size }, 1))v.push_back(Vec2Int(pos.x + p, pos.y + size));
			if (HfsmData::canFull({ pos.x + p ,pos.y - 1 }, 1))v.push_back(Vec2Int(pos.x + p, pos.y + size));
		}
		if (!v.size())return Vec2Int(0, 0);
		sort(v.begin(), v.end(),
			[=](Vec2Int a, Vec2Int b)
			{return HfsmData::dis(a, self) > HfsmData::dis(b, self); });
		return v.back();
	}
	Vec2Int finRound(Vec2Int pos, int size)
	{
		for (int p = 0; p < size; p++)
		{
			if (HfsmData::canFull({ pos.x + size ,pos.y + p }, 1))return Vec2Int(pos.x + size, pos.y + p);
			if (HfsmData::canFull({ pos.x - 1 ,pos.y + p }, 1))return Vec2Int(pos.x - 1, pos.y + p);
			if (HfsmData::canFull({ pos.x + p ,pos.y + size }, 1))return Vec2Int(pos.x + p, pos.y + size);
			if (HfsmData::canFull({ pos.x + p ,pos.y - 1 }, 1))return Vec2Int(pos.x + p, pos.y + size);
		}
		return Vec2Int(0, 0);
	}
	Vec2Int finRoundM(Vec2Int pos, int size, Vec2Int self)
	{
		std::vector<Vec2Int>v;
		for (int p = 0; p < size; p++)
		{
			if (HfsmData::canFullM({ pos.x + size ,pos.y + p }, 1))v.push_back(Vec2Int(pos.x + size, pos.y + p));
			if (HfsmData::canFullM({ pos.x - 1 ,pos.y + p }, 1))v.push_back(Vec2Int(pos.x - 1, pos.y + p));
			if (HfsmData::canFullM({ pos.x + p ,pos.y + size }, 1))v.push_back(Vec2Int(pos.x + p, pos.y + size));
			if (HfsmData::canFullM({ pos.x + p ,pos.y - 1 }, 1))v.push_back(Vec2Int(pos.x + p, pos.y + size));
		}
		if (!v.size())return Vec2Int(0, 0);
		sort(v.begin(), v.end(),
			[=](Vec2Int a, Vec2Int b)
			{return HfsmData::dis(a, self) > HfsmData::dis(b, self); });
		return v.back();
	}
	Vec2Int finRoundM(Vec2Int pos, int size)
	{
		for (int p = 0; p < size; p++)
		{
			if (HfsmData::canFullM({ pos.x + size ,pos.y + p }, 1))return Vec2Int(pos.x + size, pos.y + p);
			if (HfsmData::canFullM({ pos.x - 1 ,pos.y + p }, 1))return Vec2Int(pos.x - 1, pos.y + p);
			if (HfsmData::canFullM({ pos.x + p ,pos.y + size }, 1))return Vec2Int(pos.x + p, pos.y + size);
			if (HfsmData::canFullM({ pos.x + p ,pos.y - 1 }, 1))return Vec2Int(pos.x + p, pos.y + size);
		}
		return Vec2Int(0, 0);
	}
	int countPopulation()
	{
		int population = 0;
		for (size_t i = 0; i < playerView.entities.size(); i++)
		{
			const Entity& entity = playerView.entities[i];
			if (entity.playerId == nullptr || *entity.playerId != HfsmData::playerView.myId)continue;
			if (!entity.active)continue;
			population -= playerView.entityProperties.at(entity.entityType).populationUse;
			population += playerView.entityProperties.at(entity.entityType).populationProvide;
		}
		return population;
	}
	void init()
	{
		for (size_t i = 0; i < playerView.players.size(); i++)
		{
			const Player& playerE = playerView.players[i];
			if (playerE.id == playerView.myId)player = playerE;
		}
		TURRET = HfsmData::playerView.entityProperties.at(EntityType::BUILDER_BASE);
		BUILDER_UNIT = HfsmData::playerView.entityProperties.at(EntityType::BUILDER_UNIT);
		HOUSE = HfsmData::playerView.entityProperties.at(EntityType::HOUSE);
		MELEE_BASE = HfsmData::playerView.entityProperties.at(EntityType::MELEE_BASE);
		MELEE_UNIT = HfsmData::playerView.entityProperties.at(EntityType::MELEE_UNIT);
		RANGED_BASE = HfsmData::playerView.entityProperties.at(EntityType::RANGED_BASE);
		RANGED_UNIT = HfsmData::playerView.entityProperties.at(EntityType::RANGED_UNIT);
		RESOURCE = HfsmData::playerView.entityProperties.at(EntityType::RESOURCE);
		TURRET = HfsmData::playerView.entityProperties.at(EntityType::TURRET);
		WALL = HfsmData::playerView.entityProperties.at(EntityType::WALL);
		memset(vis, 0, sizeof(vis));
		memset(visM, 0, sizeof(visM));
		viewById.clear();
		for (size_t i = 0; i < playerView.entities.size(); i++)
		{
			const Entity& entity = playerView.entities[i];
			viewById[entity.id] = entity;
			makeFullM(entity.position, playerView.entityProperties.at(entity.entityType).size);
			if (playerView.entityProperties.at(entity.entityType).canMove)continue;
			makeFull(entity.position, playerView.entityProperties.at(entity.entityType).size);
		}
	}
}
class HfsmNode
{
public:
	Entity selfInfo;
	virtual NodeType get_type()
	{
		return NodeType::HfsmNode;
	}
	virtual std::shared_ptr<HfsmNode> get_next_state()
	{
		return nullptr;
	}
	virtual EntityAction get_action()
	{
		std::shared_ptr<MoveAction> moveAction = nullptr;
		std::shared_ptr<BuildAction> buildAction = nullptr;
		std::shared_ptr<RepairAction> repairAction = nullptr;
		std::shared_ptr<AttackAction> attackAction = nullptr;
		return EntityAction(moveAction, buildAction, attackAction, repairAction);
	}
	HfsmNode(Entity entity)
	{
		selfInfo = entity;
	}
	Vec2Int findPos()
	{
		int size = HfsmData::playerView.entityProperties.at(selfInfo.entityType).size;
		return HfsmData::finRoundM(selfInfo.position, size);
	}
};
class BuildNode : public HfsmNode
{
public:
	int timer;
	BuildNode(Entity entity, int sleepTime = 0) :HfsmNode(entity)
	{
		timer = sleepTime;
	}
};
class TurretRangerOnlyNode : public BuildNode
{
public:
	TurretRangerOnlyNode(Entity entity) :BuildNode(entity) {}
	virtual std::shared_ptr<HfsmNode> get_next_state();
	virtual EntityAction get_action()
	{
		std::shared_ptr<MoveAction> moveAction = nullptr;
		std::shared_ptr<BuildAction> buildAction = nullptr;
		std::shared_ptr<RepairAction> repairAction = nullptr;
		std::vector<EntityType> validAutoAttackTargets = { EntityType::RANGED_UNIT };
		std::shared_ptr<AttackAction> attackAction = std::shared_ptr<AttackAction>(new AttackAction(
			nullptr, std::shared_ptr<AutoAttack>(new AutoAttack(HfsmData::TURRET.sightRange, validAutoAttackTargets))));
		return EntityAction(moveAction, buildAction, attackAction, repairAction);
	}
};
class TurretAllNode : public BuildNode
{
public:
	TurretAllNode(Entity entity) :BuildNode(entity) {}
	virtual std::shared_ptr<HfsmNode> get_next_state()
	{
		for (int i = 0; i < HfsmData::playerView.entities.size(); i++)
		{
			const Entity& entityE = HfsmData::playerView.entities[i];
			if (entityE.playerId == nullptr || *entityE.playerId != HfsmData::playerView.myId)continue;
			if (entityE.entityType != EntityType::RANGED_UNIT)continue;
			if (HfsmData::dis(selfInfo.position, entityE.position) <= HfsmData::TURRET.attack->attackRange)
			{
				return std::shared_ptr<HfsmNode>(new TurretAllNode(selfInfo));
			}
		}
		return nullptr;
	}
	virtual EntityAction get_action()
	{
		std::shared_ptr<MoveAction> moveAction = nullptr;
		std::shared_ptr<BuildAction> buildAction = nullptr;
		std::vector<EntityType> validAutoAttackTargets;
		std::shared_ptr<AttackAction> attackAction = std::shared_ptr<AttackAction>(new AttackAction(
			nullptr, std::shared_ptr<AutoAttack>(new AutoAttack(HfsmData::TURRET.sightRange, validAutoAttackTargets))));
		std::shared_ptr<RepairAction> repairAction = nullptr;
		return EntityAction(moveAction, buildAction, attackAction, repairAction);
	}
};
class BaseNode : public BuildNode
{
public:
	EntityType BaseType;
	BaseNode(Entity entity) :BuildNode(entity)
	{
		BaseType = HfsmData::playerView.entityProperties.at(selfInfo.entityType).build->options[0];
	}
};
class BuilderBaseNode : public BaseNode
{
public:
	BuilderBaseNode(Entity entity) :BaseNode(entity) {}
	virtual std::shared_ptr<HfsmNode> get_next_state()
	{
		return nullptr;
	}
	virtual EntityAction get_action()
	{
		std::shared_ptr<MoveAction> moveAction = nullptr;
		std::shared_ptr<BuildAction> buildAction = std::shared_ptr<BuildAction>(new BuildAction(BaseType, findPos()));
		std::shared_ptr<RepairAction> repairAction = nullptr;
		std::shared_ptr<AttackAction> attackAction = nullptr;
		return EntityAction(moveAction, buildAction, attackAction, repairAction);
	}
};
class RangerBaseNode : public BaseNode
{
public:
	RangerBaseNode(Entity entity) :BaseNode(entity) {}
	virtual std::shared_ptr<HfsmNode> get_next_state()
	{
		return nullptr;
	}
	virtual EntityAction get_action()
	{
		std::shared_ptr<MoveAction> moveAction = nullptr;
		std::shared_ptr<BuildAction> buildAction = nullptr;
		std::shared_ptr<RepairAction> repairAction = nullptr;
		std::shared_ptr<AttackAction> attackAction = nullptr;

		if (timer <= 5)buildAction = std::shared_ptr<BuildAction>(new BuildAction(BaseType, findPos()));
		timer--;
		if (timer <= 0)timer = 20;

		return EntityAction(moveAction, buildAction, attackAction, repairAction);
	}
};
class MinerNode : public HfsmNode
{
public:
	MinerNode(Entity entity) :HfsmNode(entity) {}
	virtual NodeType get_type()
	{
		return NodeType::MinerNode;
	}
	virtual std::shared_ptr<HfsmNode> get_next_state()
	{
		return nullptr;
	}
	virtual EntityAction get_action()
	{
		std::shared_ptr<MoveAction> moveAction = nullptr;
		std::shared_ptr<BuildAction> buildAction = nullptr;
		std::vector<EntityType> validAutoAttackTargets = { EntityType::RESOURCE };
		std::shared_ptr<AttackAction> attackAction = std::shared_ptr<AttackAction>(new AttackAction(
			nullptr, std::shared_ptr<AutoAttack>(new AutoAttack(1000, validAutoAttackTargets))));
		std::shared_ptr<RepairAction> repairAction = nullptr;
		return EntityAction(moveAction, buildAction, attackAction, repairAction);
	}
};
class RepairerNode : public HfsmNode
{
public:
	int target;
	RepairerNode(Entity entity, int entityE) :HfsmNode(entity)
	{
		target = entityE;
	}
	virtual NodeType get_type()
	{
		return NodeType::Repairer;
	}
	virtual std::shared_ptr<HfsmNode> get_next_state()
	{
		if (!HfsmData::viewById.count(target))return std::shared_ptr<HfsmNode>(new MinerNode(selfInfo));
		int maxHealth = HfsmData::playerView.entityProperties.at(HfsmData::viewById[target].entityType).maxHealth;
		if (HfsmData::viewById[target].health == maxHealth)return std::shared_ptr<HfsmNode>(new MinerNode(selfInfo));
		return nullptr;
	}
	virtual EntityAction get_action()
	{
		std::shared_ptr<MoveAction> moveAction = nullptr;
		if (HfsmData::dis(selfInfo, HfsmData::viewById[target]) > 1)
		{
			Vec2Int pos=HfsmData::finRoundM(HfsmData::viewById[target].position,
				HfsmData::playerView.entityProperties.at(HfsmData::viewById[target].entityType).size,selfInfo.position);
			moveAction = std::shared_ptr<MoveAction>(new MoveAction(pos, true, true));
		}
		std::shared_ptr<BuildAction> buildAction = nullptr;
		std::shared_ptr<AttackAction> attackAction = nullptr;
		std::shared_ptr<RepairAction> repairAction = std::shared_ptr<RepairAction>(new RepairAction(target));
		return EntityAction(moveAction, buildAction, attackAction, repairAction);
	}
};
class BuilderNode : public HfsmNode
{
public:
	EntityType buildType;
	Vec2Int targetPos;
	int timer;
	BuilderNode(Entity entity, EntityType type, Vec2Int pos, int time = 10) :HfsmNode(entity)
	{
		buildType = type;
		targetPos = pos;
		timer = time;
	}
	virtual NodeType get_type()
	{
		return NodeType::BuilderNode;
	}
	virtual std::shared_ptr<HfsmNode> get_next_state()
	{
		timer--;
		if (timer <= 0 || !HfsmData::canFull(targetPos, HfsmData::playerView.entityProperties.at(buildType).size))
			return std::shared_ptr<HfsmNode>(new MinerNode(selfInfo));
		return nullptr;
	}
	virtual EntityAction get_action()
	{
		std::shared_ptr<MoveAction> moveAction = std::shared_ptr<MoveAction>(new MoveAction(
			HfsmData::finRound(targetPos, HfsmData::playerView.entityProperties.at(buildType).size), true, true));
		std::shared_ptr<BuildAction> buildAction = std::shared_ptr<BuildAction>(new BuildAction(
			buildType, targetPos));
		std::shared_ptr<AttackAction> attackAction = nullptr;
		std::shared_ptr<RepairAction> repairAction = nullptr;
		return EntityAction(moveAction, buildAction, attackAction, repairAction);
	}
};





std::shared_ptr<HfsmNode> init_node(Entity entity)
{
	if (entity.entityType == EntityType::HOUSE)
	{
		return std::shared_ptr<HfsmNode>(new BuildNode(entity));
	}
	if (entity.entityType == EntityType::TURRET)
	{
		return std::shared_ptr<HfsmNode>(new TurretAllNode(entity));
	}
	if (entity.entityType == EntityType::BUILDER_BASE)
	{
		return std::shared_ptr<HfsmNode>(new  BuilderBaseNode(entity));
	}
	if (entity.entityType == EntityType::RANGED_BASE)
	{
		return std::shared_ptr<HfsmNode>(new RangerBaseNode(entity));
	}
	if (entity.entityType == EntityType::BUILDER_UNIT)
	{
		return std::shared_ptr<HfsmNode>(new MinerNode(entity));
	}
	return std::shared_ptr<HfsmNode>(new HfsmNode(entity));
}
void repairConvene()
{
	std::vector<Entity>unhealthy;
	for (size_t i = 0; i < HfsmData::playerView.entities.size(); i++)
	{
		const Entity& entity = HfsmData::playerView.entities[i];
		if (entity.playerId == nullptr || *entity.playerId != HfsmData::playerView.myId)continue;
		if (HfsmData::playerView.entityProperties.at(entity.entityType).canMove)continue;
		if (entity.health == HfsmData::playerView.entityProperties.at(entity.entityType).maxHealth)continue;
		unhealthy.push_back(entity);
	}
	std::vector<Entity>miner;
	for (size_t i = 0; i < HfsmData::playerView.entities.size(); i++)
	{
		const Entity& entity = HfsmData::playerView.entities[i];
		if (entity.playerId == nullptr || *entity.playerId != HfsmData::playerView.myId)continue;
		if (HfsmData::hfsmStates[entity.id]->get_type() != NodeType::MinerNode)continue;
		miner.push_back(entity);
	}
	sort(unhealthy.begin(), unhealthy.end(),
		[](Entity a, Entity b) {return a.position.x + a.position.y < b.position.x + b.position.y; });
	for (Entity entity : unhealthy)
	{
		int cnt = HfsmData::playerView.entityProperties.at(entity.entityType).size;
		for (size_t i = 0; i < HfsmData::playerView.entities.size(); i++)
		{
			const Entity& entityE = HfsmData::playerView.entities[i];
			if (entityE.playerId == nullptr || *entityE.playerId != HfsmData::playerView.myId)continue;
			if (HfsmData::hfsmStates[entityE.id]->get_type() != NodeType::Repairer)continue;
			if (std::dynamic_pointer_cast<RepairerNode>(HfsmData::hfsmStates[entityE.id])->target == entity.id)cnt--;
		}
		sort(miner.begin(), miner.end(),
			[=](Entity a, Entity b) {return HfsmData::dis(entity, a) > HfsmData::dis(entity, b); });
		while (miner.size() && cnt-- > 0)
		{
			Entity entityE = miner.back();
			HfsmData::hfsmStates[entityE.id] = std::shared_ptr<HfsmNode>(new  RepairerNode(entityE, entity.id));
			miner.pop_back();
		}
	}
}
void buildConvene()
{
	if (HfsmData::player.resource < HfsmData::HOUSE.initialCost)return;
	int population = HfsmData::countPopulation();
	for (size_t i = 0; i < HfsmData::playerView.entities.size(); i++)
	{
		const Entity& entity = HfsmData::playerView.entities[i];
		if (entity.playerId == nullptr || *entity.playerId != HfsmData::playerView.myId)continue;
		if (HfsmData::hfsmStates[entity.id]->get_type() != NodeType::BuilderNode)continue;
		EntityType buildType = std::dynamic_pointer_cast<BuilderNode>(HfsmData::hfsmStates[entity.id])->buildType;
		population += HfsmData::playerView.entityProperties.at(buildType).populationProvide;
	}
	if (population >= 5)return;
	std::vector<Entity>miner;
	for (size_t i = 0; i < HfsmData::playerView.entities.size(); i++)
	{
		const Entity& entity = HfsmData::playerView.entities[i];
		if (entity.playerId == nullptr || *entity.playerId != HfsmData::playerView.myId)continue;
		if (HfsmData::hfsmStates[entity.id]->get_type() != NodeType::MinerNode)continue;
		miner.push_back(entity);
		break;
	}
	if (miner.size() == 0)return;
	for (int i = 0; i <= 20; i += 4)
	{
		for (int j = 0; j <= 20; j += 4)
		{
			if (HfsmData::canFull(Vec2Int(i, j), HfsmData::HOUSE.size))
			{
				sort(miner.begin(), miner.end(),
					[=](Entity a, Entity b)
					{return HfsmData::dis(Vec2Int(i, j), a.position) > HfsmData::dis(Vec2Int(i, j), b.position); });
				Entity entityE = miner.back();
				HfsmData::hfsmStates[entityE.id] = std::shared_ptr<HfsmNode>(new  BuilderNode(
					entityE, EntityType::HOUSE, Vec2Int(i, j)));
				miner.pop_back();
				return;
			}
		}
	}
}
void convene()
{
	repairConvene();
	buildConvene();
}
Action MyStrategy::getAction(const PlayerView& playerView, DebugInterface* debugInterface)
{
	HfsmData::playerView = playerView;
	HfsmData::init();
	for (size_t i = 0; i < playerView.entities.size(); i++)
	{
		const Entity& entity = playerView.entities[i];
		if (entity.playerId == nullptr || *entity.playerId != HfsmData::playerView.myId)continue;
		if (!HfsmData::hfsmStates.count(entity.id))
			HfsmData::hfsmStates[entity.id] = init_node(entity);
		HfsmData::hfsmStates[entity.id]->selfInfo = entity;
		auto state = HfsmData::hfsmStates[entity.id]->get_next_state();
		if (state != nullptr)HfsmData::hfsmStates[entity.id] = state;
	}
	convene();

	Action result = Action(std::unordered_map<int, EntityAction>());
	for (size_t i = 0; i < playerView.entities.size(); i++)
	{
		const Entity& entity = playerView.entities[i];
		if (entity.playerId == nullptr || *entity.playerId != HfsmData::playerView.myId)continue;
		result.entityActions[entity.id] = HfsmData::hfsmStates[entity.id]->get_action();
	}
	return result;
}

void MyStrategy::debugUpdate(const PlayerView& playerView, DebugInterface& debugInterface)
{
	debugInterface.send(DebugCommand::Clear());
	debugInterface.getState();
	debugInterface.send
	(
		DebugCommand::Add
		(
			std::shared_ptr<DebugData::Log>(new DebugData::Log("time:" + std::to_string(playerView.currentTick)))
		)
	);
}

inline std::shared_ptr<HfsmNode> TurretRangerOnlyNode::get_next_state()
{
	for (int i = 0; i < HfsmData::playerView.entities.size(); i++)
	{
		const Entity& entityE = HfsmData::playerView.entities[i];
		if (entityE.playerId == nullptr || *entityE.playerId != HfsmData::playerView.myId)continue;
		if (entityE.entityType != EntityType::RANGED_UNIT)continue;
		if (HfsmData::dis(selfInfo.position, entityE.position) > HfsmData::TURRET.attack->attackRange)
		{
			return nullptr;
		}
	}
	return std::shared_ptr<HfsmNode>(new TurretAllNode(selfInfo));
}
