#include <stdio.h>
#include "RequestHandlers.h"
#include "Request.h"

static unsigned int id_tmp=0;
/*
 * ------------------------------------------------------------------
 * add_item_handler --
 *
 *      Handle an AddItemReq.
 *
 *      Delete the request object when done.
 *
 * Results:
 *      None.
 *
 * ------------------------------------------------------------------
 */
void 
add_item_handler(void *args)
{
    // TODO: Your code here.
	unsigned int id=id_tmp++;
	AddItemReq* arg=(AddItemReq*)args;
	printf("[%d] Handling AddItem: item_id=%d, quantity=%d, price=%lf, discount=%lf\n", id, arg->item_id, arg->quantity, arg->price, arg->discount);
	arg->store->addItem(arg->item_id, arg->quantity, arg->price, arg->discount);
	printf("[%d] ends\n",id);
	delete arg;
}

/*
 * ------------------------------------------------------------------
 * remove_item_handler --
 *
 *      Handle a RemoveItemReq.
 *
 *      Delete the request object when done.
 *
 * Results:
 *      None.
 *
 * ------------------------------------------------------------------
 */
void 
remove_item_handler(void *args)
{
    // TODO: Your code here.
	unsigned int id=id_tmp++;
	RemoveItemReq* arg=(RemoveItemReq*)args;
	printf("[%d] Handling RemoveItem: item_id=%d\n", id, arg->item_id);
	arg->store->removeItem(arg->item_id);
	printf("[%d] ends\n", id);
	delete arg;
}

/*
 * ------------------------------------------------------------------
 * add_stock_handler --
 *
 *      Handle an AddStockReq.
 *
 *      Delete the request object when done.
 *
 * Results:
 *      None.
 *
 * ------------------------------------------------------------------
 */
void 
add_stock_handler(void *args)
{
    // TODO: Your code here.
	unsigned int id=id_tmp++;
	AddStockReq* arg=(AddStockReq*)args;
	printf("[%d] Handling AddStock: item_id=%d, count=%d\n", id, arg->item_id, arg->additional_stock);
	arg->store->addStock(arg->item_id, arg->additional_stock);
	printf("[%d] ends\n", id);
	delete arg;
}

/*
 * ------------------------------------------------------------------
 * change_item_price_handler --
 *
 *      Handle a ChangeItemPriceReq.
 *
 *      Delete the request object when done.
 *
 * Results:
 *      None.
 *
 * ------------------------------------------------------------------
 */
void 
change_item_price_handler(void *args)
{
    // TODO: Your code here.
	unsigned int id=id_tmp++;
	ChangeItemPriceReq* arg=(ChangeItemPriceReq*)args;
	printf("[%d] Handling ChangeItemPrice(priceItem): item_id=%d, price=%lf\n", id, arg->item_id, arg->new_price);
	arg->store->priceItem(arg->item_id, arg->new_price);
	printf("[%d] ends\n", id);
	delete arg;
}

/*
 * ------------------------------------------------------------------
 * change_item_discount_handler --
 *
 *      Handle a ChangeItemDiscountReq.
 *
 *      Delete the request object when done.
 *
 * Results:
 *      None.
 *
 * ------------------------------------------------------------------
 */
void 
change_item_discount_handler(void *args)
{
    // TODO: Your code here.
	unsigned int id=id_tmp++;
	ChangeItemDiscountReq* arg=(ChangeItemDiscountReq*)args;
	printf("[%d] Handling ChangeItemDiscount(discountItem): item_id=%d, discount=%lf\n", id, arg->item_id, arg->new_discount);
	arg->store->discountItem(arg->item_id, arg->new_discount);
	printf("[%d] ends\n", id);
	delete arg;
}

/*
 * ------------------------------------------------------------------
 * set_shipping_cost_handler --
 *
 *      Handle a SetShippingCostReq.
 *
 *      Delete the request object when done.
 *
 * Results:
 *      None.
 *
 * ------------------------------------------------------------------
 */
void 
set_shipping_cost_handler(void *args)
{
    // TODO: Your code here.
	unsigned int id=id_tmp++;
	SetShippingCostReq* arg=(SetShippingCostReq*)args;
	printf("[%d] Handling ShippingCost(setShippingCost): price=%lf\n", id, arg->new_cost);
	arg->store->setShippingCost(arg->new_cost);
	printf("[%d] ends\n", id);
	delete arg;
}

/*
 * ------------------------------------------------------------------
 * set_store_discount_handler --
 *
 *      Handle a SetStoreDiscountReq.
 *
 *      Delete the request object when done.
 *
 * Results:
 *      None.
 *
 * ------------------------------------------------------------------
 */
void
set_store_discount_handler(void *args)
{
    // TODO: Your code here.
	unsigned int id=id_tmp++;
	SetStoreDiscountReq* arg=(SetStoreDiscountReq*)args;
	printf("[%d] Handling StoreDiscount(setStoreDiscount): discount=%lf\n", id, arg->new_discount);
	arg->store->setStoreDiscount(arg->new_discount);
	printf("[%d] ends\n", id);
	delete arg;
}

/*
 * ------------------------------------------------------------------
 * buy_item_handler --
 *
 *      Handle a BuyItemReq.
 *
 *      Delete the request object when done.
 *
 * Results:
 *      None.
 *
 * ------------------------------------------------------------------
 */
void
buy_item_handler(void *args)
{
    // TODO: Your code here.
	unsigned int id=id_tmp++;
	BuyItemReq* arg=(BuyItemReq*)args;
	printf("[%d] Handling BuyItem: item_id=%d, budget=%lf\n", id, arg->item_id, arg->budget);
	arg->store->buyItem(arg->item_id, arg->budget);
	printf("[%d] ends\n", id);
	delete arg;
}

/*
 * ------------------------------------------------------------------
 * buy_many_items_handler --
 *
 *      Handle a BuyManyItemsReq.
 *
 *      Delete the request object when done.
 *
 * Results:
 *      None.
 *
 * ------------------------------------------------------------------
 */
void
buy_many_items_handler(void *args)
{
    // TODO: Your code here.
	printf("Handling BuyManyItems: not implemented yet\n");
}

/*
 * ------------------------------------------------------------------
 * stop_handler --
 *
 *      The thread should exit.
 *
 * Results:
 *      None.
 *
 * ------------------------------------------------------------------
 */
void 
stop_handler(void* args)
{
    // TODO: Your code here.
	printf("Handling StopHandler\n");
	sthread_exit();
}

