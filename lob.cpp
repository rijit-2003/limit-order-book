#include <bits/stdc++.h>
using namespace std;

enum class Side { BUY, SELL };

struct Order {
    long long id;
    Side side;
    double price;
    int quantity;
    long long timestamp;
};

class BookSide {
public:
    map<double, vector<Order>, function<bool(double,double)>> levels;

    BookSide(bool is_bid)
        : levels(is_bid
            ? function<bool(double,double)>([](double a, double b){ return a > b; }) // highest first
            : function<bool(double,double)>([](double a, double b){ return a < b; }) // lowest first
        ) {}

    void add_order(const Order& o) {
        levels[o.price].push_back(o);
    }

    void print(const string& name) const {
        cout << "--- " << name << " ---\n";
        for (auto &p : levels) {
            cout << "Price " << p.first << ": ";
            for (auto &o : p.second) {
                cout << "[id=" << o.id << ", qty=" << o.quantity << "] ";
            }
            cout << "\n";
        }
    }
};

struct Trade {
    long long buy_id;
    long long sell_id;
    double price;
    int quantity;
};

class LimitOrderBook {
public:
    BookSide bids;
    BookSide asks;
    vector<Trade> trades;

    unordered_map<long long, pair<double, vector<Order>::iterator>> order_index;

    long long ts_counter = 0;

    LimitOrderBook() : bids(true), asks(false) {}

    void add_limit_order(long long id, Side side, double price, int quantity) {
        Order incoming { id, side, price, quantity, ts_counter++ };

        if (side == Side::BUY) {
            match_buy(incoming);
            if (incoming.quantity > 0) {
                bids.add_order(incoming);
                auto &level = bids.levels[incoming.price];
                order_index[id] = { incoming.price, prev(level.end()) };
            }
        } else {
            match_sell(incoming);
            if (incoming.quantity > 0) {
                asks.add_order(incoming);
                auto &level = asks.levels[incoming.price];
                order_index[id] = { incoming.price, prev(level.end()) };
            }
        }
    }

    void add_market_order(long long id, Side side, int quantity) {
        double extreme_price = (side == Side::BUY)
            ? numeric_limits<double>::max()
            : 0.0;

        Order incoming { id, side, extreme_price, quantity, ts_counter++ };

        if (side == Side::BUY)
            match_buy(incoming);
        else
            match_sell(incoming);

        // market orders do NOT rest → no indexing needed
    }

    void cancel_order(long long id) {
        if (order_index.find(id) == order_index.end()) {
            cout << "Order " << id << " not found\n";
            return;
        }

        auto [price, it] = order_index[id];
        bool is_bid = bids.levels.count(price);

        BookSide &side = is_bid ? bids : asks;
        auto &vec = side.levels[price];

        vec.erase(it);
        if (vec.empty()) {
            side.levels.erase(price);
        }

        order_index.erase(id);

        cout << "Cancelled order " << id << "\n";
    }

    void print_book() const {
        bids.print("BIDS");
        asks.print("ASKS");
    }

    void print_trades() const {
        cout << "=== Trades ===\n";
        if (trades.empty()) {
            cout << "No trades executed yet.\n";
            return;
        }

        long long total_qty = 0;
        double total_notional = 0.0;

        for (auto &t : trades) {
            cout << "BUY " << t.buy_id
                << " matched with SELL " << t.sell_id
                << " qty=" << t.quantity
                << " @ " << t.price << "\n";
            total_qty += t.quantity;
            total_notional += t.price * t.quantity;
        }

        cout << "Total traded volume: " << total_qty << "\n";
        cout << "VWAP: " << total_notional / max(1LL, total_qty) << "\n";
    }

private:
    void match_buy(Order &incoming);
    void match_sell(Order &incoming);
};


void LimitOrderBook::match_buy(Order &incoming) {
    while (incoming.quantity > 0 && !asks.levels.empty()) {
        auto best_ask_it = asks.levels.begin();
        double best_ask_price = best_ask_it->first;

        if (best_ask_price > incoming.price)
            break;

        auto &ask_level = best_ask_it->second;
        Order &resting = ask_level.front();

        int traded = min(incoming.quantity, resting.quantity);

        trades.push_back({ incoming.id, resting.id, best_ask_price, traded });

        incoming.quantity -= traded;
        resting.quantity -= traded;

        if (resting.quantity == 0) {
            order_index.erase(resting.id);
            ask_level.erase(ask_level.begin());
            if (ask_level.empty())
                asks.levels.erase(best_ask_it);
        }
    }
}

void LimitOrderBook::match_sell(Order &incoming) {
    while (incoming.quantity > 0 && !bids.levels.empty()) {
        auto best_bid_it = bids.levels.begin();
        double best_bid_price = best_bid_it->first;

        if (best_bid_price < incoming.price)
            break;

        auto &bid_level = best_bid_it->second;
        Order &resting = bid_level.front();

        int traded = min(incoming.quantity, resting.quantity);

        trades.push_back({ resting.id, incoming.id, best_bid_price, traded });

        incoming.quantity -= traded;
        resting.quantity -= traded;

        if (resting.quantity == 0) {
            order_index.erase(resting.id);
            bid_level.erase(bid_level.begin());
            if (bid_level.empty())
                bids.levels.erase(best_bid_it);
        }
    }
}


int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    LimitOrderBook lob;

    cout << "Simple Limit Order Book\n";
    cout << "Commands:\n";
    cout << "  B id price qty   -> buy limit\n";
    cout << "  S id price qty   -> sell limit\n";
    cout << "  M B id qty       -> market buy\n";
    cout << "  M S id qty       -> market sell\n";
    cout << "  C id             -> cancel order\n";
    cout << "  P                -> print book\n";
    cout << "  T                -> print trades\n";
    cout << "  Q                -> quit\n\n";

    while (true) {
        char cmd;
        if (!(cin >> cmd)) break;

        if (cmd == 'B' || cmd == 'S') {
            long long id;
            double price;
            int qty;
            cin >> id >> price >> qty;
            lob.add_limit_order(id,
                cmd == 'B' ? Side::BUY : Side::SELL,
                price, qty);
        }
        else if (cmd == 'M') {
            char sc;
            long long id;
            int qty;
            cin >> sc >> id >> qty;
            lob.add_market_order(id, sc == 'B' ? Side::BUY : Side::SELL, qty);
        }
        else if (cmd == 'C') {
            long long id;
            cin >> id;
            lob.cancel_order(id);
        }
        else if (cmd == 'P') {
            lob.print_book();
        }
        else if (cmd == 'T') {
            lob.print_trades();
        }
        else if (cmd == 'Q') {
            break;
        }
        else {
            cout << "Unknown command\n";
        }
    }

    return 0;
}
