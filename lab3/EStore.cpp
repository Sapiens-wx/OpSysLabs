#include <cassert>
#include <cstring>
#include <stdio.h>

#include "EStore.h"

using namespace std;


Item::
Item() : valid(false)
{ }

Item::
~Item()
{ }


EStore::
EStore(bool enableFineMode)
    : fineMode(enableFineMode)
{
    // TODO: Your code here.
	store_discount=0;
	shippingCost=3;
	memset(inventory, 0, INVENTORY_SIZE*sizeof(Item));
	//init locks
	smutex_init(&lock);
	//init condition variables
	scond_init(&scond_itemEditted);
	//init [locks] if finemode enabled
	if(fineMode){
		for(int i=0;i<INVENTORY_SIZE;++i){
			smutex_init(locks+i);
		}
	}
}

EStore::
~EStore()
{
    // TODO: Your code here.
	smutex_destroy(&lock);
	//condition variables
	scond_destroy(&scond_itemEditted);
	//destroy [locks] if findMode enabled
	if(fineMode){
		for(int i=0;i<INVENTORY_SIZE;++i){
			smutex_init(locks+i);
		}
	}
}

/*
 * ------------------------------------------------------------------
 * buyItem --
 *
 *      Attempt to buy the item from the store.
 *
 *      An item can be bought if:
 *          - The store carries it.
 *          - The item is in stock.
 *          - The cost of the item plus the cost of shipping is no
 *            more than the budget.
 *
 *      If the store *does not* carry this item, simply return and
 *      do nothing. Do not attempt to buy the item.
 *
 *      If the store *does* carry the item, but it is not in stock
 *      or its cost is over budget, block until both conditions are
 *      met (at which point the item should be bought) or the store
 *      removes the item from sale (at which point this method
 *      returns).
 *
 *      The overall cost of a purchase for a single item is defined
 *      as the current cost of the item times 1 - the store
 *      discount, plus the flat overall store shipping fee.
 *
 * Results:
 *      None.
 *
 * ------------------------------------------------------------------
 */
void EStore::
buyItem(int item_id, double budget)
{
    assert(!fineModeEnabled());

    // TODO: Your code here.
	flock(item_id);
	Item& item=inventory[item_id];
	//wait for the item to statisfy all the conditions
	while(item.valid && (item.quantity==0 || getItemCost(item)>budget)){
		scond_wait(&scond_itemEditted, &lock);
	}
	//if item is not offered
	if(!item.valid){
		funlock(item_id);
		return;
	}
	//buy the item
	item.quantity--;

	funlock(item_id);
}

/*
 * ------------------------------------------------------------------
 * buyManyItem --
 *
 *      Attempt to buy all of the specified items at once. If the
 *      order cannot be bought, give up and return without buying
 *      anything. Otherwise buy the entire order at once.
 *
 *      The entire order can be bought if:
 *          - The store carries all items.
 *          - All items are in stock.
 *          - The cost of the entire order (cost of items plus
 *            shipping for each item) is no more than the budget.
 *
 *      If multiple customers are attempting to buy at the same
 *      time and their orders are mutually exclusive (i.e., the
 *      two customers are not trying to buy any of the same items),
 *      then their orders must be processed at the same time.
 *
 *      For the purposes of this lab, it is OK for the store
 *      discount and shipping cost to change while an order is being
 *      processed.
 *
 *      The cost of a purchase of many items is the sum of the
 *      costs of purchasing each item individually. The purchase
 *      cost of an individual item is covered above in the
 *      description of buyItem.
 *
 *      Challenge: For bonus points, implement a version of this
 *      method that will wait until the order can be fulfilled
 *      instead of giving up. The implementation should be efficient
 *      in that it should not wake up threads unecessarily. For
 *      instance, if an item decreases in price, only threads that
 *      are waiting to buy an order that includes that item should be
 *      signaled (though all such threads should be signaled).
 *
 *      Challenge: For bonus points, ensure that the shipping cost
 *      and store discount does not change while processing an
 *      order.
 *
 * Results:
 *      None.
 *
 * ------------------------------------------------------------------
 */
void EStore::
buyManyItems(vector<int>* item_ids, double budget)
{
    assert(fineModeEnabled());

    // TODO: Your code here.
	for(int i:*item_ids){
		flock(i);
	}
	//avoid shipping cost and store discount change
	flock(-1);

	double sum=0;
	//check can each item be bought and add the sum. or return if cannot buy the item
	for(int i:*item_ids){
		if(!inventory[i].valid || inventory[i].quantity==0){
			for(int j:*item_ids)
				funlock(j);
			funlock(-1);
			printf("buyManyItems: some item cannot be bought, not buy\n");
			return;
		}
		sum+=getItemCost(inventory[i]);
	}
	if(sum>budget){
		printf("buyManyItems: sum>budge, not buy\n");
		for(int i:*item_ids)
			funlock(i);
		funlock(-1);
		return;
	}
	for(int i:*item_ids){
		inventory[i].quantity--;
		funlock(i);
	}
	//avoid shipping cost and store discount change
	funlock(-1);
}

/*
 * ------------------------------------------------------------------
 * addItem --
 *
 *      Add the item to the store with the specified quantity,
 *      price, and discount. If the store already carries an item
 *      with the specified id, do nothing.
 *
 * Results:
 *      None.
 *
 * ------------------------------------------------------------------
 */
void EStore::
addItem(int item_id, int quantity, double price, double discount)
{
    // TODO: Your code here.
	printf("EStore.cpp: add item called\n");
	flock(item_id);

	Item& item=inventory[item_id];
	//the item already exists. do nothing
	if(item.valid){
		funlock(item_id);
		return;
	}
	item.valid=true;
	item.quantity=quantity;
	item.price=price;
	item.discount=discount;
	scond_broadcast(&scond_itemEditted, getLock(item_id));
	printf("EStore.cpp: addItem: id=%d\n", item_id);

	funlock(item_id);
}

/*
 * ------------------------------------------------------------------
 * removeItem --
 *
 *      Remove the item from the store. The store no longer carries
 *      this item. If the store is not carrying this item, do
 *      nothing.
 *
 *      Wake any waiters.
 *
 * Results:
 *      None.
 *
 * ------------------------------------------------------------------
 */
void EStore::
removeItem(int item_id)
{
    // TODO: Your code here.
	flock(item_id);

	Item& item=inventory[item_id];
	if(!item.valid){
		funlock(item_id);
		return;
	}
	item.valid=false;
	scond_broadcast(&scond_itemEditted, getLock(item_id));

	funlock(item_id);
}

/*
 * ------------------------------------------------------------------
 * addStock --
 *
 *      Increase the stock of the specified item by count. If the
 *      store does not carry the item, do nothing. Wake any waiters.
 *
 * Results:
 *      None.
 *
 * ------------------------------------------------------------------
 */
void EStore::
addStock(int item_id, int count)
{
    // TODO: Your code here.
	flock(item_id);

	Item& item=inventory[item_id];
	if(!item.valid){
		funlock(item_id);
		return;
	}
	item.quantity+=count;
	scond_broadcast(&scond_itemEditted, getLock(item_id));

	funlock(item_id);
}

/*
 * ------------------------------------------------------------------
 * priceItem --
 *
 *      Change the price on the item. If the store does not carry
 *      the item, do nothing.
 *
 *      If the item price decreased, wake any waiters.
 *
 * Results:
 *      None.
 *
 * ------------------------------------------------------------------
 */
void EStore::
priceItem(int item_id, double price)
{
    // TODO: Your code here.
	flock(item_id);

	Item& item=inventory[item_id];
	if(!item.valid){
		funlock(item_id);
		return;
	}
	item.price=price;
	scond_broadcast(&scond_itemEditted, getLock(item_id));

	funlock(item_id);
}

/*
 * ------------------------------------------------------------------
 * discountItem --
 *
 *      Change the discount on the item. If the store does not carry
 *      the item, do nothing.
 *
 *      If the item discount increased, wake any waiters.
 *
 * Results:
 *      None.
 *
 * ------------------------------------------------------------------
 */
void EStore::
discountItem(int item_id, double discount)
{
    // TODO: Your code here.
	flock(item_id);

	Item& item=inventory[item_id];
	if(!item.valid){
		funlock(item_id);
		return;
	}
	item.discount=discount;
	scond_broadcast(&scond_itemEditted, getLock(item_id));

	funlock(item_id);
}

/*
 * ------------------------------------------------------------------
 * setShippingCost --
 *
 *      Set the per-item shipping cost. If the shipping cost
 *      decreased, wake any waiters.
 *
 * Results:
 *      None.
 *
 * ------------------------------------------------------------------
 */
void EStore::
setShippingCost(double cost)
{
    // TODO: Your code here.
	flock(-1);

	bool costDec=false;
	if(shippingCost>cost)
		costDec=true;
	shippingCost=cost;
	if(costDec)
		scond_broadcast(&scond_itemEditted, getLock(-1));

	funlock(-1);
}

/*
 * ------------------------------------------------------------------
 * setStoreDiscount --
 *
 *      Set the store discount. If the discount increased, wake any
 *      waiters.
 *
 * Results:
 *      None.
 *
 * ------------------------------------------------------------------
 */
void EStore::
setStoreDiscount(double discount)
{
    // TODO: Your code here.
	flock(-1);

	bool costDec=false;
	if(store_discount>discount)
		costDec=true;
	store_discount=discount;
	if(costDec)
		scond_broadcast(&scond_itemEditted, getLock(-1));

	funlock(-1);
}

double EStore::getItemCost(const Item& item){
	return shippingCost+(1-store_discount)*(item.price*(1-item.discount));
}

void EStore::flock(int item_id){
	printf("flock(%d)\n",item_id);
	if(fineMode && item_id !=-1)
		smutex_lock(locks+item_id);
	else smutex_lock(&lock);
	printf("flock(%d) ends\n",item_id);
}

void EStore::funlock(int item_id){
	printf("funlock(%d)\n",item_id);
	if(fineMode && item_id!=-1)
		smutex_unlock(locks+item_id);
	else smutex_unlock(&lock);
	printf("funlock(%d) ends\n",item_id);
}

smutex_t* EStore::getLock(int item_id){
	if(fineMode && item_id!=-1)
		return locks+item_id;
	else return &lock;
}
