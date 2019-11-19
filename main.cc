#include "args.hh"
#include "random.hh"
#include "time_parse.hh"
#include <array>
#include <chrono>
#include <fstream>
#include <iostream>
#include <thread>
#include <vector>
#include <fmt/format.h>
#include <fmt/chrono.h>

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

    void gen_next_trade_quote(std::chrono::system_clock::time_point ticktime);
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
    auto datebeg = tqgen::time::parse(args.date_beg, "%Y%m%d"),
         dateend = tqgen::time::parse(args.date_end, "%Y%m%d"),
         starttm = tqgen::time::parse("19700101" + args.start_time, "%Y%m%d%H%M"),
         endtm = tqgen::time::parse("19700101" + args.end_time, "%Y%m%d%H%M");

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
    td.new_date = false;
    td.done = false;

    // fmt::print("Start of get_name_tick_time:\n"
    // 		"dt_now:{}, dt_eod:{}, d_now{}\n",
    // 		std::chrono::system_clock::to_time_t(this->datetime_now),
    // 		std::chrono::system_clock::to_time_t(this->datetime_eod),
    // 		std::chrono::system_clock::to_time_t(this->date_now)
    // 	);
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
    td.time = this->datetime_now;
    // fmt::print("Start of get_name_tick_time:\n"
    // 		"dt_now:{}, dt_eod:{}, d_now{}\n",
    // 		std::chrono::system_clock::to_time_t(this->datetime_now),
    // 		std::chrono::system_clock::to_time_t(this->datetime_eod),
    // 		std::chrono::system_clock::to_time_t(this->date_now)
    // 	);
    return td;
}

std::shared_ptr<stock> exch::get_next_stock() {
    // TODO: should be uniform random and not normal.
    auto p = this->stock_rand->normal(0, 1) * this->total_liquidity;
    int ctr = 0;
    while (true) {
        p -= this->stocks[ctr]->liquidity;
        if (p <= 0) {
            break;
        }
        ctr++;
    }
    return this->stocks[ctr];
}

void stock::gen_next_trade_quote(
    std::chrono::system_clock::time_point ticktime) {
    if (this->started && rnd->uniform(0, 20) > 17) {
        this->last_type = "t";
        this->last_trd_px = rnd->normal(this->last_bid_px, this->last_ask_px);
        this->last_trd_sz = (1 + rnd->uniform(0, 50)) * 100;
        this->last_trd_time = ticktime;
        this->last_arr_time =
            ticktime + std::chrono::milliseconds(rnd->uniform(0, 5) + 5);
    } else {
        this->last_type = "q";
        auto abs_spread =
            (1.0 - this->liquidity + 0.01) * this->last_trd_px / 100;
        this->last_bid_px =
            rnd->normal(this->last_trd_px - abs_spread, this->last_trd_px);
        this->last_ask_px =
            rnd->normal(this->last_trd_px, this->last_trd_px + abs_spread);
        this->last_quote_time = ticktime;
        this->last_arr_time =
            ticktime + std::chrono::milliseconds(rnd->uniform(0, 5) + 5);
        this->started = true;
    }
}

auto init_new_day_file(std::chrono::system_clock::time_point t,
                       std::string outfilepat) {
    return std::ofstream("/dev/stdout", std::ios::out);
    //    std::string fn = std::string("/dev/stdout");
    //    std::ofstream ofs(fn);
}

int main(int argc, char *argv[]) {
    parse_args(argc, argv);
    rnd = std::make_shared<tqgen::rand>(tqgen::rand(args.seed));
    auto names = gen_names(args.numstk);
    auto stocks = gen_stocks(names);
    auto exch = setup_exch(stocks.first, stocks.second);

    auto td = exch->get_next_tick_time();
    fmt::print("{}, {}, {}\n", std::chrono::system_clock::to_time_t(td.time), td.new_date, td.done);
    auto fp = init_new_day_file(td.time, args.out_file_pattern);
    fmt::print("date,arrTm,ticker,type,bidPx,bidSz,askPx,askSz,quotTm,trdPx,trdSz,"
	       "trdTm\n");
    while (!td.done) {
	auto stock = exch->get_next_stock();
	stock->gen_next_trade_quote(td.time);
	if (stock->last_type == "t") {
	    auto dnow = std::chrono::system_clock::to_time_t(exch->date_now);
	    auto larrt = std::chrono::system_clock::to_time_t(stock->last_arr_time);
	    auto ltrdt = std::chrono::system_clock::to_time_t(stock->last_trd_time);
	    fmt::print("{},{},{},{},,,,,,{},{},{}\n",
			      exch->date_now,
		       stock->last_arr_time.time_since_epoch(),
			      stock->name,
			      stock->last_type,
			      stock->last_trd_px,
			      stock->last_trd_sz,
		       stock->last_trd_time.time_since_epoch());
	} else {
	    auto dnow = std::chrono::system_clock::to_time_t(exch->date_now);
	    auto larrt = std::chrono::system_clock::to_time_t(stock->last_arr_time);
	    auto lquot = std::chrono::system_clock::to_time_t(stock->last_quote_time);
	    fmt::print("{:%Y%m%d},{:%Y%m%d%H%M%S},{},{},{},{},{},{},{:%Y%m%d%H%M%S},,,\n",
			      *std::localtime(&dnow),
			      *std::localtime(&larrt),
			      stock->name,
			      stock->last_type,
			      stock->last_bid_px,
			      stock->last_bid_sz,
			      stock->last_ask_px,
			      stock->last_ask_sz,
			      *std::localtime(&lquot));
	}
	td = exch->get_next_tick_time();
	if (td.new_date) {
	    for (auto ii: exch->stocks) {ii->started = false;}
	    // TODO: recycle file if needed
	}
    }
}
