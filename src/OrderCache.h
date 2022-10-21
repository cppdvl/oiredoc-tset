#pragma once
#include <set>
#include <string>
#include <vector>
#include <utility>
#include <algorithm>
#include <functional>
#include <unordered_map>

class Order
{
  
 public:

  // do not alter signature of this constructor
 Order(const std::string& ordId, const std::string& secId, const std::string& side, const unsigned int qty, const std::string& user,
       const std::string& company)
   : m_orderId(ordId), m_securityId(secId), m_side(side), m_qty(qty), m_user(user), m_company(company) { }

  // do not alter these accessor methods 
  std::string orderId() const    { return m_orderId; }
  std::string securityId() const { return m_securityId; }
  std::string side() const       { return m_side; }
  std::string user() const       { return m_user; }
  std::string company() const    { return m_company; }
  unsigned int qty() const       { return m_qty; }
  
  
 private:
  
  // use the below to hold the order data
  // do not remove the these member variables  
  std::string m_orderId;     // unique order id
  std::string m_securityId;  // security identifier
  std::string m_side;        // side of the order, eg Buy or Sell
  unsigned int m_qty;        // qty for this order
  std::string m_user;        // user name who owns this order
  std::string m_company;     // company for user

public:
  Order(){} /*Why!?: So we can use the [] operator*/
  Order(const Order& o) : m_orderId{o.orderId()}, m_securityId{o.securityId()}, m_side{o.side()}, m_qty{o.qty()}, m_user{o.user()}, m_company{o.company()} {}    
  Order& operator=(const Order& o) 
  { 
    m_orderId = o.orderId();
    m_securityId = o.securityId();
    m_side = o.side();
    m_qty = o.qty();
    m_user = o.user();
    m_company = o.company();
    return *this; 
  }
  Order& operator=(Order&& o)
  {
    m_orderId = std::move(o.orderId());
    m_securityId = std::move(o.securityId());
    m_side = std::move(o.side());
    m_qty = std::move(o.qty());
    m_user = std::move(o.user());
    m_company = std::move(o.company());
    return *this; 

  }

};


// Provide an implementation for the OrderCacheInterface interface class.
// Your implementation class should hold all relevant data structures you think
// are needed. 
class OrderCacheInterface
{
    
public:
  
  // implememnt the 6 methods below, do not alter signatures

  // add order to the cache
  virtual void addOrder(Order order) = 0; 

  // remove order with this unique order id from the cache
  virtual void cancelOrder(const std::string& orderId) = 0; 

  // remove all orders in the cache for this user
  virtual void cancelOrdersForUser(const std::string& user) = 0; 

  // remove all orders in the cache for this security with qty >= minQty
  virtual void cancelOrdersForSecIdWithMinimumQty(const std::string& securityId, unsigned int minQty) = 0; 

  // return the total qty that can match for the security id
  virtual unsigned int getMatchingSizeForSecurity(const std::string& securityId) = 0; 

  // return all orders in cache in a vector
  virtual std::vector<Order> getAllOrders() const = 0;  

};


using namespace std;
using UserOrdersid = unordered_map<string, set<string>>;
using SecuritiesOrdersid = unordered_map<string, vector<string>>;
using OrderidSecurity = unordered_map<string, string>;
using OrdersidOrder = unordered_map<string, Order>;

class OrderCache : public OrderCacheInterface, public OrdersidOrder 
{
  UserOrdersid user_ordersid{};

  SecuritiesOrdersid sec_ordersid{};
  OrderidSecurity orderid_sec{};

public:
  void addOrder(Order o) override
  {
    /* In order to use [] operator we need to provide a default () constructor of Order,
    however as the cache specification does not tell how to handle existing Order with
    the same orderId, we assume that edge case is handled BEFORE the addOrder function
    is called into the stack. 

    However..... a failover case is implemented to ignore any
    attempt to push in the cache any new OrderI which already exists.*/

    auto orderId = o.orderId();
    if ((*this).find(orderId) != (*this).end()) return;

    user_ordersid[o.user()].insert(orderId);
    (*this)[orderId] = o;

    //Securities mapping -- add order
    auto secId = o.securityId();
    orderid_sec[orderId] = secId;
    sec_ordersid[secId].push_back(orderId);
  }
  void cancelOrder(const std::string& orderId ) override
  {
    if ((*this).find(orderId) == (*this).end()) return;

    user_ordersid.erase((*this)[orderId].user());
    (*this).erase(orderId);

    //Securities mpping -- remove order
    {
      auto secid = orderid_sec[orderId];
      orderid_sec.erase(orderId);
      auto& secorders = sec_ordersid[secid];
      secorders.erase(remove_if(secorders.begin(), secorders.end(), [&](auto&orderid) -> bool {
        return orderid == orderId;
      }));
    }
  }
  void cancelOrdersForUser(const std::string& user) override
  {
    if (user_ordersid.find(user) == user_ordersid.end()) return;

    auto orderIds = user_ordersid[user];
    user_ordersid.erase(user);

    for (auto& orderId : orderIds)
    {
      (*this).erase(orderId);

      //Securities mpping -- remove order
      auto secid = orderid_sec[orderId];
      orderid_sec.erase(orderId);

      auto& secorders = sec_ordersid[secid];
      secorders.erase(remove_if(secorders.begin(), secorders.end(), [&](auto&orderid) -> bool {
        return orderid == orderId;
      }));
    }
  }
  void cancelOrdersForSecIdWithMinimumQty(const std::string& securityId, unsigned int minQty) override
  {
    if (sec_ordersid.find(securityId) == sec_ordersid.end()) return;

    auto orderIds = sec_ordersid[securityId];
    for (auto& orderId : orderIds)
    {
      auto removeCondition = (*this)[orderId].qty() >= minQty;
      if (!removeCondition) continue;

      user_ordersid[(*this)[orderId].user()].erase(orderId);
      (*this).erase(orderId);

      //Securities mapping -- remove order
      auto secid = orderid_sec[orderId];
      orderid_sec.erase(orderId);

      auto& secorders = sec_ordersid[secid];
      secorders.erase(remove_if(secorders.begin(), secorders.end(), [&](auto&orderid) ->bool {
        return orderid == orderId;
      }));
    }
  }
  unsigned int getMatchingSizeForSecurity(const std::string& securityId)
  {
    auto orderIds = sec_ordersid[securityId];
    for (auto& orderid : orderIds)
    {

    }
    return 0;    
  }
  vector<Order> getAllOrders()
  {
    auto allOrders = vector<Order>{};
    for (auto& kv : (*this))
    {
      allOrders.push_back(kv.second);
    }
    return allOrders;
  }
};



