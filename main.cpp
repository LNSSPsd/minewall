#include <modloader/statichook.h>
#include <modloader/loader.h>
#include <modloader/log.h>
#include <vector>
#include <string>
#include <signal.h>
#include <functional>
#include <memory>
#include <memory.h>
#include <unordered_map>
#include <mutex>

//#define ENABLE_INVENTORY_LOG

class NetworkIdentifier;
class ServerPlayer;
class ServerNetworkHandler {
public:
	ServerPlayer *_getServerPlayer(const NetworkIdentifier &nid, unsigned char subid);
};
class Packet {
};
class DisconnectPacket : public Packet {
};
class ClientCacheBlobStatusPacket : public Packet {
};

TInstanceHook(void, _ZN20ServerNetworkHandler6handleERK17NetworkIdentifierRK16DisconnectPacket, ServerNetworkHandler, NetworkIdentifier const& nid, DisconnectPacket const& packet) {
	ServerPlayer *player=this->_getServerPlayer(nid, *((unsigned char*)&packet+16+7));
	if(!player) {
		modloader::Log::warn("minewall","Handled a disconnect attack.");
		return;
	}
	return original(this, nid, packet);
}

class ReadOnlyBinaryStream {
public:
	unsigned int getUnsignedVarInt();
	uint64_t getUnsignedInt64();
};

extern "C" void _ZNSt6vectorImSaImEE12emplace_backIJmEEERmDpOT_(char *self, unsigned long *target);

TInstanceHook(bool,_ZN27ClientCacheBlobStatusPacket5_readER20ReadOnlyBinaryStream,ClientCacheBlobStatusPacket,ReadOnlyBinaryStream &stream) {
	unsigned int firstInt=stream.getUnsignedVarInt();
	unsigned int secondInt=stream.getUnsignedVarInt();
	if(firstInt+secondInt<=0xFFF) {
		if(firstInt>0xFFF||secondInt>0xFFF) {
			modloader::Log::warn("minewall", "Handled an overflow attack!");
			return false;
		}
		uint64_t tmp;
		for(;firstInt;--firstInt) {
			tmp=stream.getUnsignedInt64();
			_ZNSt6vectorImSaImEE12emplace_backIJmEEERmDpOT_((char *)this+48,&tmp);
		}
		for(;secondInt;--secondInt) {
			tmp=stream.getUnsignedInt64();
			_ZNSt6vectorImSaImEE12emplace_backIJmEEERmDpOT_((char *)this+72,&tmp);
		}
	}
	return true;
}
class Level;
class Minecraft {
public:
	Level *getLevel() const;
};

class DedicatedServer {
public:
	Minecraft *getPrimaryMinecraft();
	void stop();
};

DedicatedServer *globalServer;

TInstanceHook(void, _ZN15DedicatedServer5startERKNSt7__cxx1112basic_stringIcSt11char_traitsIcESaIcEEE, DedicatedServer, std::string const &str) {
	globalServer=this;
	signal(SIGINT,(sighandler_t)[](int signal) {
		modloader::Log::info("minewall","Handled SIGINT, stopping server");
		globalServer->stop();
	});
	original(this, str);
}

#ifdef ENABLE_INVENTORY_LOG
THook(void, _ZN21ItemTransactionLogger3logENSt7__cxx1112basic_stringIcSt11char_traitsIcESaIcEEE,std::string str) {
	modloader::Log::verbose("ItemTransactionLogger",str.c_str());
}
#endif

enum class ItemStackRequestActionType : unsigned char {
	Take,
	Place,
	Swap,
	Drop,
	Destroy,
	Consume,
	Create,
	PlaceInItemContainer,
	TakeFromItemContainer,
	ScreenLabTableCombine,
	ScreenBeaconPayment,
	ScreenHUDMineBlock,
	CraftRecipe,
	CraftRecipeAuto,
	CraftCreative,
	CraftRecipeOptional,
	CraftRepairAndDisenchant,
	CraftLoom,
	CraftNonImplemented_DEPRECATEDASKTYLAING,
	CraftResults_DEPRECATEDASKTYLAING
};
class ActorDamageSource {
public:
	ActorDamageSource(std::string const&);
};
class ItemStackRequestAction {
public:
	ItemStackRequestActionType getActionType() const;
};
enum class ActorUniqueID : int64_t {};
class MerchantRecipeList;
class EconomyTradeableComponent {
public:
	MerchantRecipeList *getOffers();
};
class SimpleContainer;
class Actor {
public:
	void hurt(ActorDamageSource const&,int,bool,bool);
	std::string const&& getNameTag() const;
	ActorUniqueID getUniqueID() const;
	Level *getLevel() const;
	EconomyTradeableComponent *getEconomyTradeableComponent() const;
	SimpleContainer &getArmorContainer();
	SimpleContainer &getHandContainer();
};
class BlockActor {
public:
	int getType() const;
};
class ContainerScreenContext;
class ContainerModel;
class ContainerManagerModel {
public:
	ContainerScreenContext *getScreenContext() const;
	std::unordered_map<std::string, std::shared_ptr<ContainerModel>> const& getContainers();
};
class Mob:public Actor {
public:
	void clearEquipment();
};
class ItemStack;
class PlayerInventory {
public:
	void removeResource(ItemStack const&);
};
class Player : public Mob {
public:
	std::string& getPlatformOnlineId() const;
	bool hasOpenContainer() const;
	void addExperience(int);
	std::shared_ptr</*IContainerManager*/ContainerManagerModel> getContainerManager();
	unsigned int /*GameType*/getPlayerGameType() const;
	//FillingContainer *getInventory() const;
	PlayerInventory &getSupplies();
};
class BlockPalette;
class Recipes;
class Level {
public:
	Player *getPlayer(ActorUniqueID) const;
	Recipes *getRecipes() const;
	BlockPalette *getBlockPalette();
};
class ServerPlayer : public Player {
public:
	void displayWhisperMessage(std::string const&,std::string const&,std::string const&,std::string const&);
	void sendNetworkPacket(Packet &) const;
	char initiateContainerClose();
	void disconnect();
	void sendInventory(bool);
};
class ContainerScreenContext {
public:
	unsigned char getScreenContainerType() const;
	BlockActor *tryGetBlockActor() const;
	Actor *tryGetActor() const;
	Player *getPlayer() const;
};
class ItemStackNetManagerBase {
	public:
	const ContainerScreenContext *getScreenContext() const;
};
class ItemStackNetManagerServer {
public:
	void tryCloseContainerScreen(std::function<void()>);
};
class ModalFormRequestPacket : public Packet {
public:
	ModalFormRequestPacket(unsigned int,std::string const &);
};

class ServerInstance {
public:
	void queueForServerThread(std::function<void()>);
};

static ServerInstance *globalServerInstance;

TInstanceHook(void, _ZN14ServerInstance17startServerThreadEv, ServerInstance) {
	globalServerInstance=this;
	original(this);
}

extern void *modloader_dlsym_print_error(const char *sym);

TInstanceHook(void, _ZN20ServerNetworkHandler6handleERK17NetworkIdentifierRK24SpawnExperienceOrbPacket, ServerNetworkHandler, NetworkIdentifier const& identifier, void *packet) {
	unsigned char subid=*(unsigned char*)((char*)packet+16);
	ServerPlayer *targetPlayer=this->_getServerPlayer(identifier, subid);
	if(!targetPlayer)return;
	int experience=*((int*)packet+14);
	modloader::Log::debug("minewall","Client requesting experience: %d",experience);
	if(experience>14) {
		targetPlayer->disconnect();
		return;
	}
	if(!targetPlayer->hasOpenContainer()) {
		modloader::Log::info("minewall","Experience orb spawn request without container opened");
		return;
	}
	BlockActor *blockActor=targetPlayer->getContainerManager()->getScreenContext()->tryGetBlockActor();
	Actor *actor=targetPlayer->getContainerManager()->getScreenContext()->tryGetActor();
	if(!blockActor&&!actor)
		return;
	targetPlayer->addExperience(experience);
	return;
}

class RecipeNetIdTag;
template <typename A,typename T,T D>
struct TypedServerNetId {
	T id=D;
	std::string toString() const;
};

class ItemDescriptor {
public:
	bool operator==(ItemDescriptor const&) const;
	ItemDescriptor();
	~ItemDescriptor();
};
class ItemDescriptorCount:public ItemDescriptor {
};
class RecipeIngredient:public ItemDescriptorCount {
};

class Recipe {
public:
	std::vector<RecipeIngredient> const& getIngredients() const;
};

class Recipes {
public:
	Recipe *getRecipeByNetId(TypedServerNetId<RecipeNetIdTag,unsigned int,0u> const&) const;
};

class NetworkItemDescriptor:public ItemDescriptor {
};

class NetworkItemInstanceDescriptor : public ItemDescriptorCount {
};

class ItemInstance;
class ItemStackRequestActionCraftResults_DEPRECATEDASKTYLAING : public ItemStackRequestAction {
public:
	std::vector<ItemInstance> const& getResults() const;
};
class Tag {
};
class CompoundTag : public Tag {
public:
	std::string toString() const;
	bool equals(Tag const&) const;
};

class ItemStackBase {
public:
	ItemDescriptor getDescriptor() const;
	bool operator==(ItemStackBase const&) const;
	bool operator!=(ItemStackBase const&) const;
	std::string toString() const;
	std::string toDebugString() const;
	CompoundTag *getUserData() const;
	bool sameItem(ItemStackBase const&) const;
	bool isEnchanted() const;
	unsigned short getId() const;
	unsigned short getAuxValue() const;
	uint32_t getIdAux() const;
	bool isDamageableItem() const;
	ItemStack& operator=(ItemStack const&);
	~ItemStackBase();
};
class ItemInstance:public ItemStackBase {
	char filler[136];
};

class MerchantRecipe {
public:
	ItemInstance *getBuyAItem() const;
	ItemInstance *getBuyBItem() const;
	ItemInstance *getSellItem() const;
};

class MerchantRecipeList {
public:
	MerchantRecipe *getRecipeByNetId(TypedServerNetId<RecipeNetIdTag,unsigned int, 0u> const&) const;
};

template <typename A, enum ItemStackRequestActionType B>
class ItemStackRequestActionCraft {
public:
	A const& getRecipeNetId() const;
};
class CreativeItemNetIdTag;
std::mutex recipesLock;
std::unordered_map<ServerPlayer *, TypedServerNetId<RecipeNetIdTag,unsigned int, 0u>> pendingRecipes;
std::unordered_map<ServerPlayer *, TypedServerNetId<CreativeItemNetIdTag,unsigned int,0>> pendingCreativeItems;

TInstanceHook(void,_ZN25ItemStackNetManagerServer18_handleRequestDataERSt6vectorI21ItemStackResponseInfoSaIS1_EEPK20ItemStackRequestData,ItemStackNetManagerServer,void *a,void *b) {
	original(this, a,b);
	ServerPlayer *player=*(ServerPlayer**)((uint64_t*)this+2);
	auto playerDidnotConsume=pendingRecipes.find(player);
	if(playerDidnotConsume!=pendingRecipes.end()) {
		// WTF? The player didn't request to consume item ?!
		player->clearEquipment();
		player->sendInventory(true);
		pendingRecipes.erase(player);
		return;
	}
}

extern "C" void _ZN14ItemDescriptorD1Ev(void *self) {
	void *des=modloader_dlsym_print_error("_ZN14ItemDescriptorD2Ev");
	void (*desc)(void*)=(void(*)(void*))des;
	desc(self);
	return;
}

/*ItemDescriptor *getItemDescriptor(const ItemStackBase *item) {
	ItemDescriptor *ret=(ItemDescriptor *)malloc(58);
	void* (*ItemStackBase_getDescriptor)(ItemDescriptor *,const ItemStackBase *)=(void* (*)(ItemDescriptor *,const ItemStackBase *))modloader_dlsym_print_error("_ZNK13ItemStackBase13getDescriptorEv");
	ItemStackBase_getDescriptor(ret,item);
	return ret;
}

void freeItemDescriptor(ItemDescriptor *a1) {
	free((void *)a1);
}*/

class CreativeItemEntry {
public:
	ItemInstance *getItemInstance() const;
};

class CreativeItemRegistry {
public:
	CreativeItemEntry *findCreativeItemEntry(TypedServerNetId<CreativeItemNetIdTag,unsigned int, 0u> const&);
	static CreativeItemRegistry *getOrCreateCurrent();
};
class ItemStackNetIdVariant;
class Item;
class ItemStack : public ItemStackBase {
	char filler[144];
public:
	ItemStack getStrippedNetworkItem() const;
	std::string toString() const;
	void removeEnchants();
	ItemStackNetIdVariant *getItemStackNetIdVariant() const;
	~ItemStack();
	ItemStack(Item const&, int);
};
class GrindstoneContainerManagerModel : ContainerManagerModel {
public:
	ItemStack const& getSlot(int32_t) const; // RESULT: 2
};

bool isEmptyItemStack(ItemStack const* stack) {
	return !*((char*)stack+34);
}

ItemStack *getStrippedNetworkItem_bridge(ItemStack const*stack) {
	ItemStack *out=(ItemStack *)malloc(200);
	((void* (*)(ItemStack*,ItemStack const*))modloader_dlsym_print_error("_ZNK9ItemStack22getStrippedNetworkItemEv"))(out,stack);
	return out;
}
ItemStack *ItemStack_clone_bridge(ItemStack const*stack) {
	ItemStack *out=(ItemStack *)malloc(200);
	((void* (*)(ItemStack*,ItemStack const*))modloader_dlsym_print_error("_ZNK9ItemStack5cloneEv"))(out,stack);
	return out;
}

ItemStack *createItemStackFromItemInstance(ItemInstance const *instance) {
	ItemStack *out=(ItemStack *)malloc(200);
	((void* (*)(ItemStack*,ItemInstance const*))modloader_dlsym_print_error("_ZN9ItemStackC2ERK12ItemInstance"))(out,instance);
	return out;
}

/*ItemStack *createAirItemStack() {
	ItemStack *out=(ItemStack *)malloc(200);
	((void* (*)(ItemStack*,ItemInstance const*))modloader_dlsym_print_error("_ZN9ItemStackC2ERK12ItemInstance"))(out,instance);
	return out;
}*/

void ItemStackBridgeFree(ItemStack *target) {
	((void(*)(ItemStack*))modloader_dlsym_print_error("_ZN9ItemStackD2Ev"))(target);
	//free(target);
}

class ContainerModel {
public:
	std::vector<ItemStack> const& getItems() const;
	unsigned int getContainerSize() const;
	void setItem(int, ItemStack const&);
	void _clearCreatedItem(int);
};

class FullPlayerInventoryWrapper {
public:
	void removeResource(ItemStack const&,bool,int);
	int getItemCount(std::function<bool(ItemStack const&)>);
};

FullPlayerInventoryWrapper *createFullPlayerInventoryWrapper(Player *player) {
	FullPlayerInventoryWrapper *out=(FullPlayerInventoryWrapper *)malloc(42);
	((void* (*)(FullPlayerInventoryWrapper *,PlayerInventory&,SimpleContainer&,SimpleContainer&,void *,Player *))modloader_dlsym_print_error("_ZN26FullPlayerInventoryWrapperC2ER15PlayerInventoryR15SimpleContainerS3_P27InventoryTransactionManagerP6Player"))(out,player->getSupplies(),player->getArmorContainer(),player->getHandContainer(),(void*)((char *)player+4672),player);
	return out;
}

class ContainerManagerController {
public:
	std::vector<ItemStack> const& getSplitItems() const;
};

bool isItemStackInvalid(ItemStack const& itemStack) {
	ItemStack *EmptyItemStack=(ItemStack *)modloader_dlsym_print_error("_ZN9ItemStack10EMPTY_ITEME");
	if(EmptyItemStack==&itemStack) {
		return true;
	}
	return false;
}

unsigned int makeCombinedID(unsigned short a,unsigned short b) {
	int32_t combinedId;
	*((unsigned short *)&combinedId)=a;
	*((unsigned short *)&combinedId+1)=b;
	return combinedId;
}

unsigned int makeCombinedID(ItemStackBase const& item) {
	return makeCombinedID(item.getId(),item.getAuxValue());
}

class Item {
public:
	~Item();
};

extern "C" void _ZN4ItemD1Ev(Item *item) {
	(*(void(**)(Item*))(*(void***)item))(item);
}

extern "C" void _ZN9ItemStackD1Ev(void *item) {
	(*(void(**)(void*))(*(void***)item))(item);
}

class ItemRegistry {
public:
	static Item getItem(short);
};

class Container {
public:
	void removeItem(int slot,int count);
	void setContainerChanged(int);
	void setItemWithForceBalance(int,ItemStack const&,bool);
};

// Not used currently
bool consumeItemInstances(Player *player, std::vector<ItemStackBase*> items) {
	//ItemStack *stack=createItemStackFromItemInstance(&item);
	//FullPlayerInventoryWrapper *wrapper=createFullPlayerInventoryWrapper(player);
	//wrapper->removeResource(*stack,true,1);
	ContainerManagerModel *containerManagerModel=*(ContainerManagerModel **)((char*)player+2952);
	auto &themap=containerManagerModel->getContainers();
	std::unordered_map<unsigned short,unsigned char> consumeNeedles;
	for(ItemStackBase *item : items) {
		consumeNeedles[makeCombinedID(*item)]=*((unsigned char*)item+34);
	}
	modloader::Log::debug("minewall","(Consume) Start");
	std::vector<Item **> &itemRegistry=*(std::vector<Item **> *)modloader_dlsym_print_error("_ZN12ItemRegistry13mItemRegistryE");
	modloader::Log::debug("minewall","(Consume) Item Created");
	ItemStack gomiStack(**itemRegistry[1], 1);
	modloader::Log::debug("VA","Created rubbish %s",gomiStack.toDebugString().c_str());
	for(auto i=themap.begin();i!=themap.end();i++) {
		if(i->second->getContainerSize()>4)continue;
		Container *container=(*(Container*(**)(ContainerModel*))(*(void***)i->second.get()+21))(i->second.get());
		modloader::Log::debug("minewall","(Consume) Container: %s",i->first.c_str());
		auto& items=i->second->getItems();
		for(int n=0;n<items.size();n++) {
			ItemStack const& item=items[n];
			unsigned char currentCount=*((unsigned char*)&item+34);
			auto srh=consumeNeedles.find(makeCombinedID(item));
			if(srh==consumeNeedles.end())continue;
			int offset=(*(int(**)(ContainerModel *))(*(void***)i->second.get()+22))(i->second.get());
			if(currentCount<=srh->second) {
				//Item airItem=ItemRegistry::getItem(0);
				//ItemStack nStack(airItem,0);
				consumeNeedles[srh->first]-=currentCount;
				// *((unsigned char*)&item+34)=0;
				// *((ItemStack *)&item)=*(ItemStack *)modloader_dlsym_print_error("_ZN9ItemStack10EMPTY_ITEME");
				//i->second->setItem(n+offset, nStack);
				//i->second->_clearCreatedItem(n+offset);
				*((unsigned char*)&gomiStack+34)=currentCount;
				*(uint32_t*)((char*)&gomiStack+136)=*(uint32_t*)item.getItemStackNetIdVariant();
				container->setItemWithForceBalance(n+offset, gomiStack, true);
				//container->removeItem(n+offset, 255);
				modloader::Log::debug("minewall","(Consume) CombineId: %d, all used",srh->first);
			}/*else{
				// *((unsigned char*)&item+34)-=srh->second;
				//container->removeItem(n+offset, srh->second);
				*((unsigned char*)&gomiStack+34)=srh->;
				container->setItemWithForceBalance(n+offset, gomiStack, true);
				unsigned int used=srh->second;
				consumeNeedles[srh->first]=0;
				//i->second->setItem(n+offset, item);
				modloader::Log::debug("minewall","(Consume) CombinedId: %d, not all used, used %d, left %d.",srh->first,used,*((unsigned char*)&item+34));
			}*/
			///container->setContainerChanged(1);
		}
		//break;
	}
	for(auto i=consumeNeedles.begin();i!=consumeNeedles.end();i++) {
		if(i->second!=0) {
			modloader::Log::debug("minewall","(Consume) Lacks item [CombinedId %d], aborting.",i->first);
			return false;
		}
	}
	modloader::Log::debug("minewall","(Consume) Finished");
	return true;
}

struct ItemStackRequestSlotInfo {
	unsigned char containerId;
	unsigned char slot;
	unsigned short a1_2_3;
	int32_t stackNetId;
	unsigned char a1_5;
	unsigned char a1_6;
	unsigned char a1_7;
	unsigned char flag; // 0 1 2 to write stack netid
};

void *buildConsume(unsigned char count,unsigned char containerId,unsigned char slot,int stackNetId) {
	struct ItemStackRequestSlotInfo info={0};
	info.containerId=containerId;
	info.slot=slot;
	info.stackNetId=stackNetId;
	info.flag=2;
	void *buf=malloc(80);
	((void *(*)(void *,unsigned char,ItemStackRequestSlotInfo const&))modloader_dlsym_print_error("_ZN29ItemStackRequestActionConsumeC2EhRK24ItemStackRequestSlotInfo"))(buf,count,info);
	return buf;
}

void freeConsume(void *consume) {
	((void (*)(void*))modloader_dlsym_print_error("_ZN29ItemStackRequestActionConsumeD0Ev"))(consume);
	//free(consume);
}

class ItemStackRequestActionTransferBase : public ItemStackRequestAction {
public:
	void *getDst() const;
};

TInstanceHook(bool, _ZN23ItemStackNetManagerBase23_isRequestActionAllowedERK22ItemStackRequestAction, ItemStackNetManagerBase, ItemStackRequestAction const&action) {
	//modloader::Log::info("minewall","ActionType: %d",action.getActionType());
	//if(action.getActionType()==ItemStackRequestActionType::CraftRecipeAuto||action.getActionType()==ItemStackRequestActionType::CraftRecipeOptional||action.getActionType()==ItemStackRequestActionType::UNK_4||action.getActionType()==ItemStackRequestActionType::UNK_5||action.getActionType()==ItemStackRequestActionType::UNK_10)return false;
	if(action.getActionType()==ItemStackRequestActionType::Consume) {
		ServerPlayer *player=*(ServerPlayer**)((uint64_t*)this+2);
		recipesLock.lock();
		pendingRecipes.erase(player);
		recipesLock.unlock();
		return original(this,action);
	}
	if(action.getActionType()==ItemStackRequestActionType::CraftRepairAndDisenchant) {
		
	}else if(action.getActionType()==ItemStackRequestActionType::CraftResults_DEPRECATEDASKTYLAING) {
		ItemStackRequestActionCraftResults_DEPRECATEDASKTYLAING *req=(ItemStackRequestActionCraftResults_DEPRECATEDASKTYLAING *)&action;
		ServerPlayer *player=*(ServerPlayer**)((uint64_t*)this+2);
		std::string const&& playerName=player->getNameTag();
		modloader::Log::debug("minewall","Handling craft results for player %s.",playerName.c_str());
		std::vector<ItemInstance> const &results=req->getResults();
		auto searchResult=pendingRecipes.find(player);
		if(searchResult==pendingRecipes.end()) {
			// CraftRecipe not sent at all
			auto csearchResult=pendingCreativeItems.find(player);
			if(csearchResult==pendingCreativeItems.end()) {
				unsigned char type=this->getScreenContext()->getScreenContainerType();
				if(type==26/*GRINDSTONE*/) {
					if(results.size()!=1) {
						modloader::Log::debug("minewall","Rejected grindstone craft for requesting multi outputs");
						return false;
					}
					const ContainerScreenContext *screenContext=this->getScreenContext();
					GrindstoneContainerManagerModel *model=*(GrindstoneContainerManagerModel **)((char*)player+2952);//screenContext->getPlayer()->getContainerManager().get();
					//ItemStack const& item=model->getSlot(2);
					//if(item==results[0]) {
					//	return original(this, action);
					//}
					if(*((char*)&results[0]+34)!=1) {
						modloader::Log::debug("minewall","Rejected grindstone craft for requesting multi outputs");
						return false;
					}
					ItemStack const *slot0=&model->getSlot(0);
					ItemStack const *slot1=&model->getSlot(1);
					if(isEmptyItemStack(slot0)) {
						ItemStack const* tmp=slot1;
						slot1=slot0;
						slot0=tmp;
					}
					if(isEmptyItemStack(slot0)) {
						modloader::Log::debug("minewall","Rejected grindstone craft since no input found");
						return false;
					}
					if(!slot0->isDamageableItem()) {
						modloader::Log::debug("minewall","Rejected grindstone craft, item isn't compatible with grindstone.");
						return false;
					}
					std::vector<ItemStackBase *> toConsume;
					toConsume.push_back((ItemStackBase*)slot0);
					if(!isEmptyItemStack(slot1)) {
						if(!slot1->isDamageableItem()) {
							modloader::Log::debug("minewall","Rejected grindstone craft, item isn't compatible with grindstone.");
							return false;
						}
						toConsume.push_back((ItemStackBase*)slot1);
						//ItemStack *slot0WithoutNBT=ItemStack_clone_bridge(slot0);
						//ItemStack *slot1WithoutNBT=ItemStack_clone_bridge(slot1);
						//slot0WithoutNBT->removeEnchants();
						//slot1WithoutNBT->removeEnchants();
						//if(*slot0WithoutNBT!=*slot1WithoutNBT) {
						//	ItemStackBridgeFree(slot0WithoutNBT);
						//	ItemStackBridgeFree(slot1WithoutNBT);
						//	return false;
						//}
						//ItemStackBridgeFree(slot0WithoutNBT);
						//ItemStackBridgeFree(slot1WithoutNBT);
						if(slot0->getId()!=slot1->getId()) {
							modloader::Log::debug("minewall","Rejected grindstone craft because inputs' kind isn't same.");
							return false;
						}
					}
					if(slot0->getId()==results[0].getId()) {
						modloader::Log::debug("minewall","Authenticated craft: grindstone: %s",results[0].toDebugString().c_str());
						//if(!consumeItemInstances(player, toConsume)) {
						//	return false;
						//}
						return original(this, action);
					}
					modloader::Log::debug("minewall","Rejected grindstone craft for unexpected item output");
					//modloader::Log::debug("minewall","StrippedS0: %s,\nResult 0: %s",strippedS0->toDebugString().c_str(),results[0].toDebugString().c_str());
					//modloader::Log::debug("VA","0: %s,\n1: %s,\n2: %s",slot0.toString().c_str(),slot1.toString().c_str(),item.toString().c_str());
				}
				return false;
			}
			unsigned int gameType=player->getPlayerGameType();
			pendingCreativeItems.erase(player);
			if(gameType!=1&&gameType!=4) {
				modloader::Log::debug("minewall","Rejected Creative Item Request because the player's gametype isn't in creative mode.");
				return false;
			}
			TypedServerNetId<CreativeItemNetIdTag,unsigned int, 0u> itemId=csearchResult->second;
			CreativeItemRegistry *registry=CreativeItemRegistry::getOrCreateCurrent();
			if(results.size()!=1) {
				modloader::Log::debug("minewall","Rejected Creative Item Request since the result is too much.");
				return false;
			}
			if(!(*registry->findCreativeItemEntry(itemId)->getItemInstance()==results[0])) {
				modloader::Log::debug("minewall","Rejected Creative Item Request for unexpected output item.");
				return false;
			}
			modloader::Log::debug("minewall","Authenticated craft: creative get: %s",results[0].toDebugString().c_str());
			return original(this, action);
		}
		TypedServerNetId<RecipeNetIdTag,unsigned int, 0u> craftId=searchResult->second;
		Recipes *recipes=player->getLevel()->getRecipes();
		Recipe *recipe=recipes->getRecipeByNetId(craftId);
		if(!recipe) {
			std::string trans=craftId.toString();
			//modloader::Log::debug("VA","N craftId: %s",trans.c_str());
			//modloader::Log::debug("VA","No Global Recipe for recipe id %d",craftId.id);
			if(results.size()!=1) {
				recipesLock.lock();
				pendingRecipes.erase(player);
				recipesLock.unlock();
				modloader::Log::debug("minewall","Rejected an actor transaction like request for requesting multi output.");
				return false;
			}
			// May be a villager
			Actor *targetActor=this->getScreenContext()->tryGetActor();
			if(!targetActor) {
				recipesLock.lock();
				pendingRecipes.erase(player);
				recipesLock.unlock();
				// No recipe but no interacting actor
				modloader::Log::debug("minewall","Rejected craft request, recipe not found.");
				return false;
			}
			EconomyTradeableComponent *comp=targetActor->getEconomyTradeableComponent();
			if(!comp) {
				recipesLock.lock();
				pendingRecipes.erase(player);
				recipesLock.unlock();
				// Not tradeable (?)
				modloader::Log::debug("minewall","Rejected craft request, actor isn't tradeable.");
				return false;
			}
			MerchantRecipeList *recipeList=comp->getOffers();
			if(!recipeList) {
				recipesLock.lock();
				pendingRecipes.erase(player);
				recipesLock.unlock();
				modloader::Log::debug("minewall","Rejected craft request, actor isn't tradeable.");
				return false;
			}
			MerchantRecipe *recipe=recipeList->getRecipeByNetId(craftId);
			if(!recipe) {
				// Fake or invalid NetId
				recipesLock.lock();
				pendingRecipes.erase(player);
				recipesLock.unlock();
				modloader::Log::debug("minewall","Rejected craft result, invalid NetId (maybe a fake one)");
				return false;
			}
			ItemInstance *sell=recipe->getSellItem();
			if(!(*sell==results[0])) {
				// Fake recipe
				recipesLock.lock();
				pendingRecipes.erase(player);
				recipesLock.unlock();
				modloader::Log::debug("minewall","Rejected craft result, unexpected output.");
				return false;
			}
			modloader::Log::debug("minewall","Authenticated craft: trade: %s",results[0].toDebugString().c_str());
			return original(this,action);
		}
		//Item *resultItem=((Item *(*)())recipe+7)();// virtual -> getResultItem
		//modloader::Log::debug("VA","1");
		std::vector<ItemInstance> const& realResults=(*((std::vector<ItemInstance> const& (**)(Recipe*))(*(void***)recipe+5)))(recipe);// virtual getResultItem(void);
		//modloader::Log::debug("VA","Size: %d",results.size());
		if(results.size()!=realResults.size()) {
			recipesLock.lock();
			pendingRecipes.erase(player);
			recipesLock.unlock();
			modloader::Log::debug("minewall","Rejected craft, recipes not equal.");
			//modloader::Log::debug("VA","Not equal: %d, %d",results.size(),realResults.size());
			return false;
		}
		//modloader::Log::debug("VA","Final auth");
		bool currentSessionOK=false;
		for(ItemInstance const &i : realResults) {
			//modloader::Log::debug("VA","2");
			//ItemDescriptor &c=*getItemDescriptor(&i);//i.getDescriptor();
			//modloader::Log::debug("VA","3");
			for(ItemInstance const &d:results) {
				//modloader::Log::debug("VA","4");
				if(i==d) {
					//modloader::Log::debug("VA","5");
					modloader::Log::debug("minewall","Authenticated craft: craft[part]: %s",i.toDebugString().c_str());
					currentSessionOK=true;
					break;
				}
				//modloader::Log::debug("VA","6");
			}
			//freeItemDescriptor(&c);
			if(!currentSessionOK) {
				recipesLock.lock();
				pendingRecipes.erase(player);
				recipesLock.unlock();
				modloader::Log::debug("minewall","Rejected craft for unexpected output.");
				modloader::Log::debug("minewall","Wanted: [List, %d items]",realResults.size());
				for(ItemInstance const&wantedI : realResults) {
					modloader::Log::debug("minewall","%s",wantedI.toDebugString().c_str());
				}
				modloader::Log::debug("minewall","Got: [List, %d items]",results.size());
				for(ItemInstance const &d:results) {
					modloader::Log::debug("minewall","%s",d.toDebugString().c_str());
				}
				modloader::Log::info("minewall","== REJECT [%s] ==",playerName.c_str());
				return false;
			}
			currentSessionOK=false;
		}
		modloader::Log::debug("minewall","Authenticated craft: craft all pass");
		//return original(this,action);
		// Use at consume
		// With bugs unresolved, use carefully.
		//std::vector<RecipeIngredient> const& ingredients = recipe->getIngredients();
		//std::vector<ItemStackBase *> toConsume;
		//for(RecipeIngredient const&i:ingredients) {
		//	toConsume.push_back((ItemStackBase *)&i);
		//}
		//if(!consumeItemInstances(player, toConsume)) {
		//	return false;
		//}
		return original(this, action);
		
	}else if(action.getActionType()==ItemStackRequestActionType::CraftRecipe) {
		ItemStackRequestActionCraft<TypedServerNetId<RecipeNetIdTag,unsigned int, 0u>,(ItemStackRequestActionType)12> *craft=(ItemStackRequestActionCraft<TypedServerNetId<RecipeNetIdTag,unsigned int, 0u>,(ItemStackRequestActionType)12> *)&action;
		TypedServerNetId<RecipeNetIdTag,unsigned int, 0u> const& craftId=craft->getRecipeNetId();
		std::string trans=craftId.toString();
		//modloader::Log::debug("VA","Original craftId: %s",trans.c_str());
		ServerPlayer *player=*(ServerPlayer**)((uint64_t*)this+2);
		recipesLock.lock();
		pendingRecipes[player]=craftId;
		recipesLock.unlock();
	}else if(action.getActionType()==ItemStackRequestActionType::CraftCreative) {
		ItemStackRequestActionCraft<TypedServerNetId<CreativeItemNetIdTag,unsigned int, 0u>,(ItemStackRequestActionType)14> *cr=(ItemStackRequestActionCraft<TypedServerNetId<CreativeItemNetIdTag,unsigned int, 0u>,(ItemStackRequestActionType)14> *)&action;
		TypedServerNetId<CreativeItemNetIdTag,unsigned int, 0u> const& itemId=cr->getRecipeNetId();
		ServerPlayer *player=*(ServerPlayer**)((uint64_t*)this+2);
		recipesLock.lock();
		pendingCreativeItems[player]=itemId;
		recipesLock.unlock();
	}
	if(action.getActionType()!=ItemStackRequestActionType::Swap)return original(this,action);
	unsigned char type=this->getScreenContext()->getScreenContainerType();
	//modloader::Log::info("minewall","Screen: %d",(int)type);
	const ContainerScreenContext *screenContext=this->getScreenContext();
	if(type!=2/*FURNACE*/&&type!=4/*BREWING_STAND*/&&type!=27/*BLAST_FURNACE*/&&type!=28/*SMOKER*/) {
		return original(this, action);
	}
	ServerPlayer *player=*(ServerPlayer**)((uint64_t*)this+2);
	//screenContext->getPlayer()->hurt(ActorDamageSource("Cheating"),10,1,0);
	//ServerPlayer *player=(ServerPlayer*)screenContext->getPlayer();
	//player->hurt(ActorDamageSource("Cheating"),10,1,1);
	//player->initiateContainerClose();
	player->addExperience(-3);
	//((ItemStackNetManagerServer *)this)->tryCloseContainerScreen([](){});
	modloader::Log::info("minewall","Handled furnace output swapping hack");
	return false;
}



