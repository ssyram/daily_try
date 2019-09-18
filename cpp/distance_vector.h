//
//  distance_vector.h
//  Try
//
//  Created by 潇湘夜雨 on 2019/6/3.
//  Copyright © 2019 ssyram. All rights reserved.
//

#ifndef distance_vector_h
#define distance_vector_h

#include <iostream>
#include <iomanip>
#include "semaphore.h"
#include <string>
#include <thread>
#include <vector>
#include <queue>
#include <unordered_map>
#include <unordered_set>
#include <regex>
#include <functional>
using std::cout;
using std::cin;
using std::endl;
using std::regex;
using std::unordered_set;
using std::unordered_map;
using std::mutex;
using std::lock_guard;
using std::queue;
using std::vector;
using utils::semaphore;
using std::string;
using std::pair;

struct printer {
	std::ostream &os;
	template <typename ...Args>
	printer &operator()(Args &&...args) {
		(os << ... << args) << endl;
		return *this;
	}
};
printer print{ cout };

class DisVecPoint {
    // functional
public:
    using distance_t = size_t;
private:
    char _name;
    using d_map_t = unordered_map<DisVecPoint *, pair<distance_t, DisVecPoint *>>;
    d_map_t dmap;
    queue<pair<d_map_t, DisVecPoint *>> update_queue;
    unordered_set<DisVecPoint *> neighbour;
    
    // implementation
    semaphore wake_up_sem;
    mutex mtx_update_q;
    bool cont = false;
    
    void update(d_map_t to_update, DisVecPoint *p) {
        lock_guard<mutex> guard(mtx_update_q);
        
        update_queue.emplace(std::move(to_update), p);
        
        wake_up_sem.signal();
    }
    
    // should only be the terminal state, should not be called outside
    void add_neighbour() { }
    
public:
    
    struct Printer {
        static mutex mtx;
        
        static void print_wake(const DisVecPoint *p) {
            lock_guard<mutex> guard(mtx);
            
			print("Point[", p->_name, "] woke up.");
        }
        
        static void print_update(const DisVecPoint *p) {
            lock_guard<mutex> guard(mtx);
            
			print("Point[", p->_name, "] updated!");
            inner_print_vec(p);
            print("--------------------------------------");
        }
        
        static void print_vec(const DisVecPoint *p) {
            lock_guard<std::mutex> guard(mtx);
            
            inner_print_vec(p);
        }
        
        static void print_stop(const DisVecPoint *p) {
            lock_guard<mutex> guard(mtx);

			print("Point[", p->_name, "] stop.");
        }
    private:
        static void inner_print_vec(const DisVecPoint *p) {
            print
			("Point[", p->_name, "] vec:")
            ("------------------------")
            ("|name|distance|next hop|")
            ("------------------------");
			for (const auto &tp : p->dmap)
				print
				("|", tp.first->_name, "   |"
					, std::setw(8), tp.second.first, "|"
					, tp.second.second->_name, "       |");
			print
			("------------------------");
        }
    };
    
    DisVecPoint(char name): _name(name) { }
    
    const unordered_set<DisVecPoint *> &get_neighbour() const { return neighbour; }
    
    void add_neighbour(DisVecPoint *p, distance_t d) {
        neighbour.insert(p);
        dmap[p] = std::make_pair(d, p);
        p->neighbour.insert(this);
        p->dmap[this] = std::make_pair(d, this);
    }
    
    void add_neighbour(DisVecPoint *p) {
        add_neighbour(p, ~distance_t(0));
    }
    
    template <typename SecTp, typename ...Args>
    void add_neighbour(DisVecPoint *p, SecTp &&d, Args &&...args) {
        if constexpr (std::is_convertible_v<SecTp, distance_t>) {
            add_neighbour(p, distance_t(d));
            add_neighbour(std::forward<Args>(args)...);
        }
        else {
            add_neighbour(p);
            add_neighbour(std::forward<SecTp>(d), std::forward<Args>(args)...);
        }
    }
    
    void start_distance_vector() {
        // robust
        if (cont) return;
        cont = true;
        std::thread t([this]() {
            for (auto &np: neighbour)
                np->update(dmap, this);
            
            for (wake_up_sem.wait(); cont; wake_up_sem.wait()) {
                // update myself
                Printer::print_wake(this);
                auto &dmap_point_pair = update_queue.front();
                distance_t neighbour_dist = dmap[dmap_point_pair.second].first;
                bool change = false;
                for (auto &np: dmap_point_pair.first) {
                    if (np.first->_name == _name) continue;
                    auto it = dmap.find(np.first);
                    if (it == dmap.end()) {
                        dmap.emplace_hint
                        (it, np.first, std::make_pair
                         (np.second.first + neighbour_dist, dmap_point_pair.second));
                        change = true;
                    }
                    else if (it->second.first > np.second.first + neighbour_dist) {
                        it->second = std::make_pair(np.second.first + neighbour_dist, dmap_point_pair.second);
                        change = true;
                    }
                }
                if (change) {
                    Printer::print_update(this);
                    for (auto &np: neighbour)
                        np->update(dmap, this);
                }
                update_queue.pop();
            }
            Printer::print_stop(this);
        });
        t.detach();
    }
    
    char name() { return _name; }
    void stop() {
        // robust
        if (!cont) return;
        cont = false;
        wake_up_sem.signal();
    }
};

mutex DisVecPoint::Printer::mtx;

void real_run(const unordered_map<char, DisVecPoint *> &pset);

template <typename ...Args>
void run_packet(Args &&...args) {
    unordered_map<char, DisVecPoint *> pset;
    (pset.emplace(args->name(), args), ...);
    real_run(pset);
}

void real_run(const unordered_map<char, DisVecPoint *> &pset) {
    bool has_started = false;
    auto print_menu = []() {
        print
		("start: Start (Must not start yet).")
        ("print: Print Specific Point content.")
        ("menu: Print menu again.")
        ("stop: Stop the running process.")
		("quit: Quit the proces.");
    };
    auto print_names = [&pset]() {
        print("Names list: ");
        for (auto &p: pset)
            cout << p.first << ", ";
        cout << endl;
    };
    auto start = [&pset]() {
        for (auto &p: pset)
            p.second->start_distance_vector();
    };
    auto all_stop = [&pset]() {
        for (auto &p: pset)
            p.second->stop();
    };
    print_menu();
    string s;
    for (std::getline(cin, s); s != "quit"; std::getline(cin, s)) {
        if (s == "start") {
            if (has_started)
                print("Already Started.");
            else {
                has_started = true;
                start();
            }
        }
        else if (s == "print") {
            print_names();
            print("Input: (Enter 'quit' to quit this mode)");
            do {
                std::getline(cin, s);
                if (s == "quit") break;
                auto it = pset.find(s[0]);
                if (s.size() != 1 || it == pset.end())
                    print("Wrong input, please enter again.");
                else
                    DisVecPoint::Printer::print_vec(it->second);
            } while (true);
            print("exit print mode");
        }
        else if (s == "menu") print_menu();
        else if (s == "stop") {
            if (!has_started) print("Not yet started.");
            else {
                has_started = false;
                all_stop();
            }
        }
        else print("Wront input");
    }
    all_stop();
}

void get_input_and_run() {
    print
    ("Input the graph in format of Link List.")
    ("Points should all has one single English character as its name.")
    ("Format: ")
    ("<First Line>: all node names divided by <SP>(' ')")
    ("<Following Line>: <Node name> The list of Node name and distance")
    ("Followed by an empty line to finish.")
    ("Note that this graph should be connected and Repeat can be avoid.")
    ("Example: ")
    ("t u v w x y z")
    ("t    y 7 u 2 v 4")
    ("u    v 3 w 3")
    ("v    w 4 x 3 y 8")
    ("w    x 6")
    ("x    y 6 z 8")
    ("y    z 12")
    ();
    cout << "<First Line>: ";
    string s;
    static regex first_line_rgx("^[a-zA-Z](\\s+[a-zA-Z])*\\s*$");
    static regex following_rgx("^[a-zA-Z](\\s+[a-zA-Z]\\s+\\d+)+$");
    unordered_map<char, DisVecPoint *> pset;
#define except(s)\
    print(s);\
    return;
#define isNum(n) unsigned(n - '0') < 10
#define checkPointExist(idx)\
    auto it = pset.find(s[idx]);\
    if (it == pset.end()) {\
        except(((string("Point named ") += s[idx]) += " is NOT a Registered Point."))\
    }
    
    // business logic
    std::getline(cin, s);
    if (!std::regex_match(s, first_line_rgx)){
        except("First Line format error.")
    }
    for (char c: s) {
        if (c == ' ') continue;
        
        if (!pset.insert_or_assign(c, new DisVecPoint(c)).second) {
            except("Double name error.")
        }
    }
    print("Following Line(s)");
    for (std::getline(cin, s); !s.empty(); std::getline(cin, s)) {
        // format check
        if (!std::regex_match(s, following_rgx)) {
            except("Following Line format error.")
        }
        
        // first position
        // fp check
        checkPointExist(0)
        DisVecPoint *p = it->second;
        
        // following tackle
        size_t tmpDis = 0;
        DisVecPoint *tmpPnt = nullptr;
        for (size_t i = 1; i < s.size(); ++i) {
            if (s[i] == ' ') continue;
            
            if (isNum(s[i])) {
                do
                    tmpDis = tmpDis * 10 + (s[i] - '0');
                while (isNum(s[++i]));
                p->add_neighbour(tmpPnt, tmpDis);
                --i;
                tmpDis = 0;
                tmpPnt = nullptr;
            }
            else {
                checkPointExist(i)
                
                tmpPnt = it->second;
            }
        }
    }
    {
        unordered_set<const DisVecPoint *> ps;
        std::function<void (DisVecPoint *)> dfs = [&ps, &dfs](const DisVecPoint *p) {
            if (!ps.emplace(p).second) return;
            for (auto &np: p->get_neighbour())
                dfs(np);
        };
        dfs(pset.begin()->second);
        if (ps.size() != pset.size()) {
            except("This is not a connected graph")
        }
    }
    
    real_run(pset);
#undef checkPointExist
#undef isNum
#undef except
	for (auto &p: pset)
		delete p.second;
}

// two ways to run this program
// 1. to just interpret and execute keyboard input from terminal
// 2. to reuse the code
void run() {
    // the main program to run to interpret keyboard input
    get_input_and_run();
    
    // normal way to incorporate the code to run
//    DisVecPoint t('t'), u('u'), v('v'), w('w'), x('x'), y('y'), z('z');
//    t.add_neighbour(&y, 7, &u, 2, &v, 4);
//    u.add_neighbour(&v, 3, &w, 3);
//    v.add_neighbour(&w, 4, &x, 3, &y, 8);
//    w.add_neighbour(&x, 6);
//    x.add_neighbour(&y, 6, &z, 8);
//    y.add_neighbour(&z, 12);
//
//    run_packet(&t, &u, &v, &w, &x, &y, &z);
}

#endif /* distance_vector_h */
