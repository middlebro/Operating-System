//
//  main.cpp
//  Designing_a_Virtual_Memory_Manager
//
//  Created by 서형중 on 07/06/2019.
//  Copyright © 2019 Hyeongjung Seo. All rights reserved.
//

#include <iostream>
#include <fstream>
#include <cstring>
#include <vector>
#include <cmath>
#include <tuple>
#include <algorithm>
using namespace std;

int cnt_LRU = 0;
int cnt_LRUA = 0;

int bin_to_dec(vector<int> bin);            // binary to (int)dec
unsigned int bin_to_dec_u(vector<int> bin); // binary to (u_int)dec

vector<int> dec_to_bin(int dec);            // decimal to vector<int> binary
vector<int> dec_to_bin_32(unsigned int dec);// decimal to vector<int> binary

vector<int> ref_bit_increase(vector<int> ref_bits, bool is_ref);

pair<int, int> paging(int page_number, pair<int, int> *frame_table) {
    int frame_number = -1;
    for (int i = 0; i < 256; ++i) {
        if (frame_table[i].first == page_number) {
            frame_number = i;
            break;
        }
    }
    if (frame_number == -1) {
        for (int i = 0; i < 256; ++i) {
            if (frame_table[i].first == -1) {
                frame_table[i].first = page_number;
                frame_number = i;
                break;
            }
        }
    }
    return make_pair(frame_number, page_number);
}



/* Address translation
  
  logical address -> physical address
  
  logical address <page#, offset>
 
 1. TLB table 에 찾고자 하는 page# 가 있는 지 확인
 1.1 EXIST -> 해당 page 의 counter 0 으로 초기화
 1.1.1 return frame# * 256 + offset
 1.2 NOT EXIST -> frame table 에서 해당 page 탐색
 1.2.1 EXIST
 1.2.1.1 return pair<frame#, page#>
 1.2.2 NOT EXIST
 1.2.2.1 empty frame 에 해당 page 할당
 1.2.2.2 return pair<frame#, page#>
 1.2.3 TLB IS FULL?
 1.2.3.1 TRUE
 1.2.3.1.1 TLB 에서 counter 가 가장 큰 tuple 을 선택 후 추방
 1.2.3.2 빈 공간에 frame table 에서 반환된 pair 할당 및 해당 tuple 의 counter 0 으로 세팅
 1.2.3.3 나머지 tuple 의 counter 를 하나 올려줌
 2. return frame_number * 256 + offset
 
 
 
  1. TLB table 에 찾고자 하는 page#가 있는 지 확인
  1.1 NOT EXIST
  1.1.1 PAGE TABLE 에 찾고자 하는 page# 가 있는 지 확인 : 해당 pair를 TLB table에 update
  1.1.1.1 NOT EXIST
  1.1.1.1.1 PAGE TABLE -> IS FULL ?
  1.1.1.1.1.1 LRU 를 통해 victim을 선택하여 frame 공간을 확보
  1.1.1.1.2 비어있는 frame에 page# 할당
  1.1.1.1.3 할당된 pair return
  1.1.1.2 EXIST
  1.1.1.2.1 해당 pair return
  1.1.2 반환된 pair 를 TLB table 에 update
  1.1.2.1 TLB TABLE -> IS FULL?
  1.1.2.1.1 LRU 를 이용해서 victim 선택 후 TLB TABLE 공간 확보
  1.1.1.2 비어있는 공간에 pair upate
  1.2 return pair
  2. return pair->frame# + offset
*/

int Address_translator(int page_number, int offset, tuple<int, int, int> *TLB_table, pair<int, int> *frame_table) {
    int physical_address = -1;
    bool IS_FULL = false;
    int cnt = 0;
    for (int i = 0; i < 32; ++i) {
        if (get<1>(TLB_table[i]) == page_number) {
            ++cnt_LRU;
            physical_address = get<0>(TLB_table[i]);   // TLB hit
            tuple<int, int, int> tmp = make_tuple(get<0>(TLB_table[i]), get<1>(TLB_table[i]), 0);   // init counter
            TLB_table[i].swap(tmp);
            break;
        }
        if (get<1>(TLB_table[i]) != -1)
            ++cnt;
    }
    if (cnt == 32)
        IS_FULL = true;
    if (physical_address == -1) {   // TLB miss
        pair<int, int> update_target = paging(page_number, frame_table);
        physical_address = update_target.first;
        if (IS_FULL) {
            tuple<int, int, int> victim = TLB_table[0];
            int victim_idx = 0;
            for (int i = 0; i < 32; ++i) {
                if (get<2>(victim) <= get<2>(TLB_table[i])) {
                    victim = TLB_table[i];
                    victim_idx = i;
                }
            }
            tuple<int, int, int> tmp = make_tuple(get<0>(TLB_table[victim_idx]), -1, 0);
            TLB_table[victim_idx].swap(tmp);
            for (int i = 0; i < 32; ++i) {
                if (get<1>(TLB_table[i]) != -1) {
                    tuple<int, int, int> tmp = make_tuple(get<0>(TLB_table[i]), get<1>(TLB_table[i]), ++get<2>(TLB_table[i]));
                    TLB_table[i].swap(tmp);
                }
            }
        }
        for (int i = 0; i < 32; ++i) {
            if (get<1>(TLB_table[i]) == -1) {
                tuple<int, int, int> tmp = make_tuple(update_target.first, update_target.second, 0);
                TLB_table[i].swap(tmp);
                break;
            }
        }
    }
    return physical_address * 256 + offset;
}

/* Address translation
 
 logical address -> physical address
 
 logical address <page#, offset>
 1. TLB table 에 찾고자 하는 page#가 있는 지 확인
 1.1 NOT EXIST
 1.1.1 PAGE TABLE 에 찾고자 하는 page# 가 있는 지 확인 : 해당 pair를 TLB table에 update
 1.1.1.1 NOT EXIST
 1.1.1.1.1 PAGE TABLE -> IS FULL ?
 1.1.1.1.1.1 LRU 를 통해 victim을 선택하여 frame 공간을 확보
 1.1.1.1.2 비어있는 frame에 page# 할당
 1.1.1.1.3 할당된 pair return
 1.1.1.2 EXIST
 1.1.1.2.1 해당 pair return
 1.1.2 반환된 pair 를 TLB table 에 update
 1.1.2.1 TLB TABLE -> IS FULL?
 1.1.2.1.1 LRU 를 이용해서 victim 선택 후 TLB TABLE 공간 확보
 1.1.1.2 비어있는 공간에 pair upate
 1.2 return pair
 2. return pair->frame# + offset
 */

int Address_translator_A(int page_number, int offset, tuple<int, int, unsigned int> *TLB_table_A, pair<int, int> *frame_table) {
    int frame_number = -1;
    bool IS_FULL = false;
    int cnt = 0;
    for (int i = 0; i < 32; ++i) {
        if (get<1>(TLB_table_A[i]) == page_number) {
            ++cnt_LRUA;
            frame_number = get<0>(TLB_table_A[i]);   // TLB hit
            vector<int> ref_bits = dec_to_bin_32(get<2>(TLB_table_A[i]));
            ref_bits = ref_bit_increase(ref_bits, true);
            unsigned int _ref_bits = bin_to_dec_u(ref_bits);
            tuple<int, int, unsigned int> tmp = make_tuple(get<0>(TLB_table_A[i]), get<1>(TLB_table_A[i]), _ref_bits);   // init counter
            swap(TLB_table_A[i], tmp);
        }
        else if (get<1>(TLB_table_A[i]) != -1) {
            vector<int> ref_bits = dec_to_bin_32(get<2>(TLB_table_A[i]));
            ref_bits = ref_bit_increase(ref_bits, false);
            unsigned int _ref_bits = bin_to_dec_u(ref_bits);
            tuple<int, int, unsigned int> tmp = make_tuple(get<0>(TLB_table_A[i]), get<1>(TLB_table_A[i]), _ref_bits);   // init counter
            swap(TLB_table_A[i], tmp);
        }
        if (get<1>(TLB_table_A[i]) == -1)
            ++cnt;
    }
    if (cnt == 0)
        IS_FULL = true;
    if (frame_number == -1) {   // TLB miss
        pair<int, int> update_target = paging(page_number, frame_table);
        frame_number = update_target.first;
        if (IS_FULL) {
            tuple<int, int, unsigned int> victim = TLB_table_A[0];
            int victim_idx = 0;
            for (int i = 1; i < 32; ++i) {
                if (get<2>(victim) >= get<2>(TLB_table_A[i])) {
                    victim = TLB_table_A[i];
                    victim_idx = i;
                }
            }
            tuple<int, int, unsigned int> tmp = make_tuple(-1, -1, 0);
            swap(TLB_table_A[victim_idx], tmp);
        }
        for (int i = 0; i < 32; ++i) {
            if (get<1>(TLB_table_A[i]) == -1) {
                tuple<int, int, unsigned int> tmp = make_tuple(update_target.first, update_target.second, 2147483648);
                swap(TLB_table_A[i], tmp);
                break;
            }
        }
    }
    return frame_number * 256 + offset;
}

int main(int argc, const char * argv[]) {
    // LRU
    pair<int, int> frame_table[256];// <index : frame#, page#, cnt>
    tuple<int, int, int> TLB_table[32];   // <frame#, page#, cnt>
    // LRUA
    pair<int, int> frame_table_A[256];
    tuple<int, int, unsigned int> TLB_table_A[32] = {{-1, -1, 0}};   // <frame#, page#, cnt>
    
    memset(frame_table, -1, sizeof(frame_table));
    memset(TLB_table, -1, sizeof(TLB_table));
    memset(frame_table_A, -1, sizeof(frame_table_A));
    memset(TLB_table_A, -1, sizeof(TLB_table_A));
    
    // initialize page table
    ifstream logical_address_input("address.txt", ios::in);
    ofstream physical_address_output("physical.txt", ios::out);

    if (logical_address_input.is_open()) {
        string line;
        
        while (getline(logical_address_input, line,'\n')) {
            int logical_address = atoi(line.c_str());
            vector<int> binary = dec_to_bin(logical_address);
            vector<int> page_number(8);
            vector<int> offset(8);
            for (int i = 0; i < 8; ++i) {
                page_number[i] = binary[i];
            }
            for (int i = 0; i < 8; ++i) {
                offset[i] = binary[i + 8];
            }
            int _page_number = bin_to_dec(page_number);
            int _offset = bin_to_dec(offset);

            int physical_address = Address_translator(_page_number, _offset, TLB_table, frame_table);
            Address_translator_A(_page_number, _offset, TLB_table_A, frame_table_A);
            physical_address_output << to_string(physical_address) << "\n";
//            pair<int, int> frame_number = LRU(bin_to_dec(page_number), frame_table);
//            physical_address_output << to_string(frame_number.first) << "\n";
        }
    }

    // create frame_table.txt
    ofstream frame_table_output("frame_table.txt", ios::out);
    // < frame#, flag, page# > free : flag(0), allocated : flag(1)
    for (int i = 0; i < 256; ++i) {
        if (frame_table[i].first != -1)
            frame_table_output << to_string(i) << " " << "1" << " " << to_string(frame_table[i].first) << "\n";
        else
            frame_table_output << to_string(i) << " " << "0" << " " << to_string(frame_table[i].first) << "\n";
    }
    
    // create TLB_LRU.txt
    ofstream TLB_LRU_output("TLB_LRU.txt", ios::out);
    // <page#, frame#>
    for (int i = 0; i < 32; ++i) {
        TLB_LRU_output << to_string(get<1>(TLB_table[i])) << " " << to_string(get<0>(TLB_table[i])) << "\n";
    }
    
    // create TLB_LRUA.txt
    ofstream TLB_LRUA_output("TLB_LRUA.txt", ios::out);
    // <page#, frame#>
    for (int i = 0; i < 32; ++i) {
        TLB_LRUA_output << to_string(get<1>(TLB_table_A[i])) << " " << to_string(get<0>(TLB_table_A[i])) << "\n";
    }
    
    cout << "TLB_LRU : " << cnt_LRU << endl;
    cout << "TLB_LRUA : " << cnt_LRUA << endl;

    TLB_LRUA_output.close();
    TLB_LRU_output.close();
    frame_table_output.close();
    physical_address_output.close();
    logical_address_input.close();
    
    return 0;
}

vector<int> dec_to_bin(int dec) {
    vector<int> bin(16, 0);
    for (int i = 15; dec > 0; --i) {
        bin[i] = dec % 2;
        dec /= 2;
    }
    return bin;
}

int bin_to_dec(vector<int> bin) {
    int dec = 0;
    reverse(bin.begin(), bin.end());
    for (int i = 0; i < bin.size(); ++i) {
        dec += pow(2, i) * bin[i];
    }
    return dec;
}

unsigned int bin_to_dec_u(vector<int> bin) {
    unsigned int dec = 0;
    reverse(bin.begin(), bin.end());
    for (int i = 0; i < bin.size(); ++i) {
        dec += pow(2, i) * bin[i];
    }
    return dec;
}

vector<int> dec_to_bin_32(unsigned int dec) {
    vector<int> bin(32, 0);
    for (int i = 31; dec > 0; --i) {
        bin[i] = dec % 2;
        dec /= 2;
    }
    
    return bin;
}

vector<int> ref_bit_increase(vector<int> ref_bits, bool is_ref) {
    for (int i = (int)ref_bits.size() - 1; i > 0; --i) {
        ref_bits[i] = ref_bits[i - 1];
    }
    if (is_ref)
        ref_bits[0] = 1;
    else
        ref_bits[0] = 0;
    return ref_bits;
}
