#include <random>
#include <chrono>
#include <vector>
#include "MyStrategy.hpp"
#include <exception>
std::random_device rd;
std::mt19937 gen(rd()); 
template <typename T>
T rand(T l, T r)
{
	std::uniform_int_distribution<T> u(l, r);
	return u(gen);
}
MyStrategy::MyStrategy() {}
Vec2Int randPos(Vec2Int pos, const PlayerView& playerView)
{
	Vec2Int goal = { std::min(std::max(0,pos.x + rand(-1,1)),playerView.mapSize - 1),std::min(std::max(0,pos.y + rand(-1,1)),playerView.mapSize - 1) };
	if (std::min(playerView.currentTick, playerView.mapSize) > goal.x + goal.y)goal.x += 1, goal.y += 1;
	return goal;
}
int dis(Entity x, Entity y)
{
	return abs(x.position.x - y.position.x) + abs(x.position.y - y.position.y);
}
int dis(Vec2Int x, Vec2Int y)
{
	return abs(x.x - y.x) + abs(x.y - y.y);
}

int HC = 0;
int fee;
Entity posMe;
bool cmp(Entity x, Entity y)
{
	return dis(x, posMe) < dis(y,posMe);
}
Action MyStrategy::getAction(const PlayerView& playerView, DebugInterface* debugInterface)
{
	Action result = Action(std::unordered_map<int, EntityAction>());
	int myId = playerView.myId;
	for (size_t i = 0; i < playerView.players.size(); i++){
		const Player& player = playerView.players[i];
		if (player.id == myId){
			fee = player.resource;
		}
	}
	Vec2Int target = {10,10};
	int dist = 999;
	std::vector<Entity>unhealthy;
	HC = 0;
	for (size_t i = 0; i < playerView.entities.size(); i++) {
		const Entity& entity = playerView.entities[i];
		if (entity.playerId != nullptr && *entity.playerId == myId) {
			HC -= playerView.entityProperties.at(entity.entityType).populationUse;
			HC += playerView.entityProperties.at(entity.entityType).populationProvide;
			if (entity.entityType == EntityType::BUILDER_UNIT){
			}
			else if (entity.entityType == EntityType::MELEE_UNIT) {
			}
			else if (entity.entityType == EntityType::RANGED_UNIT) {
			}
			else if(entity.health!= playerView.entityProperties.at(entity.entityType).maxHealth){
				unhealthy.push_back(entity);
			}
		}
	}
	for (size_t i = 0; i < playerView.entities.size(); i++) {
		const Entity& entity = playerView.entities[i];
		if (entity.playerId == nullptr) {
			continue;
		}
		else if (*entity.playerId != myId) {
			for (size_t j = 0; j < playerView.entities.size(); j++) {
				const Entity& entityMe = playerView.entities[j];
				if (entityMe.playerId == nullptr || *entityMe.playerId != myId)continue;
				if (dist > dis(entity, entityMe)) {
					dist = dis(entity, entityMe);
					target = entity.position;
				}
			}
		}
	}
	for (size_t i = 0; i < playerView.entities.size(); i++) {
		const Entity& entity = playerView.entities[i];
		if (entity.playerId == nullptr|| *entity.playerId != myId) {
			continue;
		}
		const EntityProperties& properties = playerView.entityProperties.at(entity.entityType);

		std::shared_ptr<MoveAction> moveAction = nullptr;
		std::shared_ptr<BuildAction> buildAction = nullptr;
		std::shared_ptr<RepairAction> repairAction = nullptr;
		if (properties.canMove) {
			if (entity.entityType != EntityType::BUILDER_UNIT){
				moveAction = std::shared_ptr<MoveAction>(new MoveAction(
					target,
				    true,
					true));
			}
			else{
				if (unhealthy.size()&&dis(entity,*--unhealthy.end())<10) {
					repairAction = std::shared_ptr<RepairAction>(new RepairAction((--unhealthy.end())->id));
					moveAction = std::shared_ptr<MoveAction>(new MoveAction(
						{(--unhealthy.end())->position.x,(--unhealthy.end())->position.y},
						true,
						true));
				}
				else if (HC<5&&playerView.entityProperties.at(EntityType::HOUSE).initialCost<=fee){
					fee -= playerView.entityProperties.at(EntityType::HOUSE).initialCost;
					buildAction = std::shared_ptr<BuildAction>(new BuildAction(
						EntityType::HOUSE,
						Vec2Int(entity.position.x + playerView.entityProperties.at(entity.entityType).size, entity.position.y + playerView.entityProperties.at(entity.entityType).size - 1)));
				}
				/*
				else if (playerView.entityProperties.at(EntityType::TURRET).initialCost <= fee) {
					fee -= playerView.entityProperties.at(EntityType::TURRET).initialCost;
					buildAction = std::shared_ptr<BuildAction>(new BuildAction(
						EntityType::TURRET,
						Vec2Int(std::max(10,entity.position.x + playerView.entityProperties.at(entity.entityType).size), std::max(10,entity.position.y + playerView.entityProperties.at(entity.entityType).size - 1))));
				}*/
				else{
					Vec2Int target = { playerView.mapSize - 1, playerView.mapSize - 1 };
					for (int j = 0; j < playerView.entities.size(); j++)
					{
						const Entity& entityR = playerView.entities[j];
						if (entityR.playerId == nullptr) {
							if(dis(entityR.position,entity.position)<dis(target,entity.position)){
								target = entityR.position;
							}
						}
					}
					moveAction = std::shared_ptr<MoveAction>(new MoveAction(
						target,
						true,
						true));
				}
			}
		}
		else if (properties.build != nullptr) {
			EntityType entityType = properties.build->options[0];
			if (playerView.currentTick<100&&entityType != EntityType::BUILDER_UNIT)continue;
			if (playerView.entityProperties.at(entityType).populationUse <= HC) {
				buildAction = std::shared_ptr<BuildAction>(new BuildAction(
					entityType,
					Vec2Int(entity.position.x + properties.size, entity.position.y + properties.size - 1)));
			}
			
		}
		posMe = entity;
		std::vector<EntityType> validAutoAttackTargets;
		if (entity.entityType == BUILDER_UNIT&&repairAction==nullptr) {
			validAutoAttackTargets.push_back(RESOURCE);
		}
		result.entityActions[entity.id] = EntityAction(
			moveAction,
			buildAction,
			std::shared_ptr<AttackAction>(new AttackAction(
				nullptr, std::shared_ptr<AutoAttack>(new AutoAttack(properties.sightRange, validAutoAttackTargets)))),
			repairAction);
	}

	return result;
}

void MyStrategy::debugUpdate(const PlayerView& playerView, DebugInterface& debugInterface)
{
	debugInterface.send(DebugCommand::Clear());
	debugInterface.getState();
	debugInterface.send(DebugCommand::Add(std::shared_ptr<DebugData::Log>(new DebugData::Log(
		"time:" + std::to_string(playerView.currentTick) + "\n" +
		"mapsize:" + std::to_string(playerView.mapSize) + "\n"+
		"HC:" + std::to_string(HC) + "\n"+
		"fee:" + std::to_string(fee) + "\n"
	))));
}