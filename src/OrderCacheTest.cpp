#include "OrderCache.h"
#include "json.hpp"
using jsn = nlohmann::json;

jsn createJsonOrder (Order o)
{
  jsn j;
  j["orderid"] = o.orderId();
  j["securityid"] = o.securityId();
  j["side"] = o.side();
  j["qty"] = o.qty();
  j["user"] = o.user();
  j["company"] = o.company();
  return j;

}

jsn createJsonOrder (const std::string& ordId, const std::string& secId, const std::string& side, const unsigned int qty, const std::string& user, const std::string& company)
{
  jsn j;
  j["orderid"] = ordId;
  j["securityid"] = secId;
  j["side"] = side;
  j["qty"] = qty;
  j["user"] = user;
  j["company"] = company;
  return j;
}


bool AddOrderTest(vector<Order> os)
{
  jsn jtarget; 

  jtarget.push_back(createJsonOrder("OrdId1", "SecId1", "Sell", 100, "User10", "Company2"));
  jtarget.push_back(createJsonOrder("OrdId2", "SecId3", "Sell", 200, "User8", "Company2"));
  jtarget.push_back(createJsonOrder("OrdId3", "SecId1", "Buy", 300, "User13", "Company2"));
  jtarget.push_back(createJsonOrder("OrdId4", "SecId2", "Sell", 400, "User12", "Company2"));
  jtarget.push_back(createJsonOrder("OrdId5", "SecId3", "Sell", 500, "User7", "Company2"));
  jtarget.push_back(createJsonOrder("OrdId6", "SecId3", "Buy", 600, "User3", "Company1"));
  jtarget.push_back(createJsonOrder("OrdId7", "SecId1", "Sell", 700, "User10", "Company2"));
  jtarget.push_back(createJsonOrder("OrdId8", "SecId1", "Sell", 800, "User2", "Company1"));
  jtarget.push_back(createJsonOrder("OrdId9", "SecId2", "Buy", 900, "User6", "Company2"));
  jtarget.push_back(createJsonOrder("OrdId10", "SecId2", "Sell", 1000, "User5", "Company1"));
  jtarget.push_back(createJsonOrder("OrdId11", "SecId1", "Sell", 1100, "User13", "Company2"));
  jtarget.push_back(createJsonOrder("OrdId12", "SecId2", "Buy", 1200, "User9", "Company2"));
  jtarget.push_back(createJsonOrder("OrdId13", "SecId1", "Sell", 1300, "User1", "Company"));

  OrderCache oc;

  //Perform Action
  for (auto& o: os)
  {
    oc.addOrder(o);
  }

  for (auto& j: jtarget)
  {
    auto orderid_undertest = j["orderid"];
    /* Check the order is in the cache */
    auto orderFound = oc.find(orderid_undertest) != oc.end();
    if (!orderFound) 
    {
      cout << string{ __FILE__} + ":" << __LINE__ << std::string{" order not found in cache "} << endl;
      return false;
    }

    auto jdump = j.dump();
    auto cachedump = createJsonOrder(oc[orderid_undertest]).dump();

    if (jdump != cachedump)
    {
      cout << string{__FILE__} + ":" << __LINE__ << std::string{" orderid under test: "} << orderid_undertest << endl;
      return false;
    }
  }

  return true;
}

bool CancelOrderTest(vector<Order> os)
{
  OrderCache oc;
  //Perform Action
  for (auto& o: os)
  {
    oc.addOrder(o);
  }

  //Perform Action
  for (auto& o: os)
  {
    auto oid = o.orderId();
    oc.cancelOrder(oid);
    if (oc.find(oid) != oc.end())
    {
      cout << string{__FILE__} + ":" << __LINE__ << std::string{"order cancelling didn't take place: "} << oid << endl;
      return false;
    }
  }

  return true;
}

bool CancelOrderForUserTest(vector<Order> os)
{
  OrderCache oc;
  //Perform Action
  for (auto& o: os)
  {
    oc.addOrder(o);
  }
  

  auto user = os[0].user();
  auto ordersbyuser = oc.getUserOrders(user);

  //Perform action
  oc.cancelOrdersForUser(user);
  if (any_of(ordersbyuser.begin(), ordersbyuser.end(), [&](auto& oid) -> bool
    {
      if(oc.find(oid) != oc.end())
      {
        cout << string{__FILE__} + ": " << __LINE__ << std::string{"order cancelling didn't take place: "} << oid << endl;
        return true;
      }
      return false;
    }))
  {
    return false;
  }
  return true;
}

bool CancelOrdersForSecIdWithMinimumQtyTest(vector<Order> os)
{
  OrderCache oc; 

  for (auto& o:os)
  {
    oc.addOrder(o);
  }


  auto secs = oc.getSecs();
  for(auto&sec:secs)
  {
    oc.cancelOrdersForSecIdWithMinimumQty(sec, 1500);
  }

  //All orders should remain
  if (any_of(os.begin(), os.end(), [&](auto&o) -> bool
  {
    if (oc.find(o.orderId()) == oc.end())
    {
      cout << string{__FILE__} + string{": "} << __LINE__ << std::string{" orderid not found: "} << o.orderId() << endl;
      return true;
    }
    return false;
  }))
  {
    return false;
  }

  for (auto&sec:secs)
  {
    oc.cancelOrdersForSecIdWithMinimumQty(sec, 0);
  }

  //NO orders should remain
  if (any_of(os.begin(), os.end(), [&](auto&o) -> bool
  {
    if (oc.find(o.orderId()) != oc.end())
    {
      cout << string{__FILE__} + string{": "} << __LINE__ << std::string{" orderid found: "} << o.orderId() << endl;
      return true;
    }
    return false;
  }))
  {
    return false;
  }
  return true;

}

bool GetMatchingSizeForSecurityTest(vector<Order> matchTestOs, const std::string& secId, unsigned int qtyMatchTest)
{
  OrderCache oc;
  for (auto& o : matchTestOs)
  {
    oc.addOrder(o);
  }

  auto qtyMatch = oc.getMatchingSizeForSecurity(secId);
  auto testOk = qtyMatchTest == qtyMatch;
  if (!testOk)
  {
    cout << string{__FILE__} + string{": "} << __LINE__ << std::string{" bad matched qty: "} << qtyMatch << endl;
    return false;
  }
  return true;
}

int main ()
{
  vector<Order> os
  {
    {"OrdId1", "SecId1", "Sell", 100, "User10", "Company2"},
    {"OrdId2", "SecId3", "Sell", 200, "User8", "Company2"},
    {"OrdId3", "SecId1", "Buy", 300, "User13", "Company2"},
    {"OrdId4", "SecId2", "Sell", 400, "User12", "Company2"},
    {"OrdId5", "SecId3", "Sell", 500, "User7", "Company2"},
    {"OrdId6", "SecId3", "Buy", 600, "User3", "Company1"},
    {"OrdId7", "SecId1", "Sell", 700, "User10", "Company2"},
    {"OrdId8", "SecId1", "Sell", 800, "User2", "Company1"},
    {"OrdId9", "SecId2", "Buy", 900, "User6", "Company2"},
    {"OrdId10", "SecId2", "Sell", 1000, "User5", "Company1"},
    {"OrdId11", "SecId1", "Sell", 1100, "User13", "Company2"},
    {"OrdId12", "SecId2", "Buy", 1200, "User9", "Company2"},
    {"OrdId13", "SecId1", "Sell", 1300, "User1", "Company"}
  };

  cout << (AddOrderTest(os) ? "[OK]" : "[FAILED]") << " addOrder()" << endl;
  cout << (CancelOrderTest(os) ? "[OK]" : "[FAILED]") << " cancelOrder()" << endl;
  cout << (CancelOrderForUserTest(os) ? "[OK]" : "[FAILED]") << " cancelOrderForUser()" << endl;
  cout << (CancelOrdersForSecIdWithMinimumQtyTest(os) ? "[OK]" : "[FAILED]") << " cancelOrderForSecIdWithMinimumQty()" << endl;


  //According to what's explained the match size for SecId2 is 2700...
  vector<Order> matchTestOs0
  {
    {"OrdId1", "SecId1", "Buy",  1000, "User1", "CompanyA"},
    {"OrdId2", "SecId2", "Sell", 3000, "User2", "CompanyB"},
    {"OrdId3", "SecId1", "Sell",  500, "User3", "CompanyA"},
    {"OrdId4", "SecId2", "Buy",   600, "User4", "CompanyC"},
    {"OrdId5", "SecId2", "Buy",   100, "User5", "CompanyB"},
    {"OrdId6", "SecId3", "Buy",  1000, "User6", "CompanyD"},
    {"OrdId7", "SecId2", "Buy",  2000, "User7", "CompanyE"},
    {"OrdId8", "SecId2", "Sell", 5000, "User8", "CompanyE"}
  };



  cout << (GetMatchingSizeForSecurityTest(matchTestOs0, "SecId2", 2700) ? "[OK]" : "[FAILED]") << " getMatchingSizeForSecurity(SecId2, 2700)" << endl;

  cout << (GetMatchingSizeForSecurityTest(matchTestOs0, "SecId1", 0) ? "[OK]" : "[FAILED]") << " getMatchingSizeForSecurity(SecId1, 0)" << endl;

  vector<Order> matchTestOs1
  {

    {"OrdId1", "SecId1", "Sell", 100, "User10", "Company2"},
    {"OrdId2", "SecId3", "Sell", 200, "User8", "Company2"},
    {"OrdId3", "SecId1", "Buy", 300, "User13", "Company2"},
    {"OrdId4", "SecId2", "Sell", 400, "User12", "Company2"},
    {"OrdId5", "SecId3", "Sell", 500, "User7", "Company2"},
    {"OrdId6", "SecId3", "Buy", 600, "User3", "Company1"},
    {"OrdId7", "SecId1", "Sell", 700, "User10", "Company2"},
    {"OrdId8", "SecId1", "Sell", 800, "User2", "Company1"},
    {"OrdId9", "SecId2", "Buy", 900, "User6", "Company2"},
    {"OrdId10", "SecId2", "Sell", 1000, "User5", "Company1"},
    {"OrdId11", "SecId1", "Sell", 1100, "User13", "Company2"},
    {"OrdId12", "SecId2", "Buy", 1200, "User9", "Company2"},
    {"OrdId13", "SecId1", "Sell", 1300, "User1", "Company"}
  };

  cout << (GetMatchingSizeForSecurityTest(matchTestOs1, "SecId1", 300) ? "[OK]" : "[FAILED]") << " getMatchingSizeForSecurity(SecId1, 300)" << endl;

  cout << (GetMatchingSizeForSecurityTest(matchTestOs1, "SecId2", 1000) ? "[OK]" : "[FAILED]") << " getMatchingSizeForSecurity(SecId2, 1000)" << endl;

  cout << (GetMatchingSizeForSecurityTest(matchTestOs1, "SecId3", 600) ? "[OK]" : "[FAILED]") << " getMatchingSizeForSecurity(SecId3, 600)" << endl;


  vector<Order> matchTestOs2
  {
    {"OrdId1", "SecId3", "Sell", 100, "User1", "Company1"},
    {"OrdId2", "SecId3", "Sell", 200, "User3", "Company2"},
    {"OrdId3", "SecId1", "Buy", 300, "User2", "Company1"},
    {"OrdId4", "SecId3", "Sell", 400, "User5", "Company2"},
    {"OrdId5", "SecId2", "Sell", 500, "User2", "Company1"},
    {"OrdId6", "SecId2", "Buy", 600, "User3", "Company2"},
    {"OrdId7", "SecId2", "Sell", 700, "User1", "Company1"},
    {"OrdId8", "SecId1", "Sell", 800, "User2", "Company1"},
    {"OrdId9", "SecId1", "Buy", 900, "User5", "Company2"},
    {"OrdId10", "SecId1", "Sell", 1000, "User1", "Company1"},
    {"OrdId11", "SecId2", "Sell", 1100, "User6", "Company2"}
  };

  cout << (GetMatchingSizeForSecurityTest(matchTestOs2, "SecId2", 600) ? "[OK]" : "[FAILED]") << " getMatchingSizeForSecurity(SecId2, 600)" << endl;

  cout << (GetMatchingSizeForSecurityTest(matchTestOs2, "SecId3", 0) ? "[OK]" : "[FAILED]") << " getMatchingSizeForSecurity(SecId3, 0)" << endl;

  return 0;
}