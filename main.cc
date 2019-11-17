#include "args.hh"
#include "random.hh"
#include "time_parse.hh"
#include <array>
#include <chrono>
#include <iostream>
#include <thread>
#include <vector>

std::shared_ptr<tqgen::rand> rnd;

// clang-format off
struct stock {
    std::string					name;
    std::double_t				liquidity;
    std::double_t				base_px;
    std::chrono::system_clock::time_point	last_arr_time;
    std::string					last_type;
    std::double_t				last_bid_px;
    std::int64_t				last_bid_sz;
    std::double_t				last_ask_px;
    std::int64_t				last_ask_sz;
    std::chrono::system_clock::time_point	last_quote_time;
    std::double_t				last_trd_px;
    std::int64_t				last_trd_sz;
    std::chrono::system_clock::time_point	last_trd_time;
    bool					started;
};

struct tick_data {
    std::chrono::system_clock::time_point	time;
    bool					new_date;
    bool					done;
};

struct exch {
    std::chrono::system_clock::time_point	date_beg;
    std::chrono::system_clock::time_point	date_end;
    std::chrono::system_clock::time_point	date_now;
    std::chrono::system_clock::time_point	datetime_now;
    std::chrono::system_clock::time_point	datetime_eod;
    std::chrono::system_clock::duration		sod;
    std::chrono::system_clock::duration		eod;
    std::vector<std::shared_ptr<stock>>		stocks;
    std::double_t				total_liquidity;

    std::shared_ptr<tqgen::rand> tick_rand;
    std::shared_ptr<tqgen::rand> stock_rand;

    tick_data get_next_tick_time();
    std::shared_ptr<stock> get_next_stock();
};
// clang-format on

constexpr std::string_view gen_name(tqgen::rand &r) {
    char ret[5] = {};
    constexpr std::array<char, 26> charArray = {
        'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J', 'K', 'L', 'M',
        'N', 'O', 'P', 'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X', 'Y', 'Z'};

    for (int ii = 0; ii < 4; ii++) {
        ret[ii] = charArray[r.uniform<int>(0, 26)];
    }
    ret[4] = 0;
    return std::string_view(ret);
}

std::set<std::string> gen_names(int n) {
    std::set<std::string> res;
    tqgen::rand r(tqgen::rand::get_seed());
    while (res.size() < n) {
        res.emplace(gen_name(r));
    }
    return res;
}

auto gen_stocks(const std::set<std::string> &names) {
    std::vector<std::shared_ptr<stock>> stocks;
    tqgen::rand r(tqgen::rand::get_seed());
    tqgen::rand nr_liq(r.uniform());
    tqgen::rand nr_bpx(r.uniform());
    double total_liq = 0;
    for (auto name : names) {
        auto item = std::make_shared<stock>();
        item->name = name;
        item->liquidity = nr_liq.normal(0, 1);
        item->base_px = nr_liq.normal(0, 100);
        item->last_trd_px = item->base_px;
        item->started = false;

        total_liq += item->liquidity;
        stocks.push_back(item);
    }
    return std::pair(stocks, total_liq);
}

auto setup_exch(const std::vector<std::shared_ptr<stock>> &stocks,
                std::double_t total_liq) {
    auto datebeg = tqgen::time::parse(args.date_beg, "%Y%b%d"),
         dateend = tqgen::time::parse(args.date_end, "%Y%b%d"),
         starttm = tqgen::time::parse(args.start_time, "%H%M"),
         endtm = tqgen::time::parse(args.end_time, "%H%M");

    typedef std::chrono::duration<int, std::chrono::hours> hour_t;
    typedef std::chrono::duration<int, std::chrono::minutes> minute_t;

    auto datenow = datebeg, datetimenow = datenow + starttm.time_since_epoch(),
         datetimeeod = datenow + endtm.time_since_epoch();

    auto ex = std::make_shared<exch>();
    ex->date_beg = datebeg;
    ex->date_end = dateend;
    ex->date_now = datenow;
    ex->datetime_now = datetimenow;
    ex->datetime_eod = datetimeeod;
    ex->sod = starttm.time_since_epoch();
    ex->eod = endtm.time_since_epoch();
    ex->stocks = stocks;
    ex->total_liquidity = total_liq;

    ex->tick_rand = std::make_shared<tqgen::rand>(tqgen::rand(rnd->uniform()));
    ex->stock_rand = std::make_shared<tqgen::rand>(tqgen::rand(rnd->uniform()));
    return ex;
}

tick_data exch::get_next_tick_time() {
    tick_data td;

    this->datetime_now +=
        std::chrono::milliseconds(this->tick_rand->uniform(1, args.interval));
    if (this->datetime_now > this->datetime_eod) {
        this->date_now = this->date_now + std::chrono::hours(24);
        this->datetime_now = this->date_now + this->sod;
        this->datetime_eod = this->date_now + this->eod;
        td.new_date = true;
    }
    if (this->date_now > this->date_end) {
        td.done = true;
    }
    return td;
}

std::shared_ptr<stock> exch::get_next_stock() {
    // TODO: should be uniform random and not normal.
    auto p = this->stock_rand->normal(0, 1) * this->total_liquidity;
    int ctr = 0;
    while (true) {
	p -= this->stocks[ctr]->liquidity;
	if (p <= 0) { break; }
	ctr++;
    }
    return this->stocks[ctr];
}

int main(int argc, char *argv[]) {
    parse_args(argc, argv);
    rnd = std::make_shared<tqgen::rand>(tqgen::rand(args.seed));
    auto names = gen_names(args.numstk);
    auto stocks = gen_stocks(names);
    for (int ii = 0; ii < stocks.first.size(); ii++) {
        std::cout << stocks.first[ii]->liquidity << "\n";
    }
    std::cerr << names.size() << "\n";
}
