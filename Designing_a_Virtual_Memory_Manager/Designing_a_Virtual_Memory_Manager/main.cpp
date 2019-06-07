//
//  main.cpp
//  Designing_a_Virtual_Memory_Manager
//
//  Created by 서형중 on 07/06/2019.
//  Copyright © 2019 Hyeongjung Seo. All rights reserved.
//

#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <cmath>
#define _max(x,y) ((x > y) ? x : y)
using namespace std;

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

struct Node {
    int data;
    Node *up, *down;
    Node() {
        data = NULL;
        up = down = NULL;
    }
    Node(int _data) : data(_data) {
        up = down  = NULL;
    }
};

class stack {
private:
    Node *TOP, *BOTTOM;
    int size;
public:
    stack() {
        TOP = new Node();
        BOTTOM = new Node();
    }
    stack(int _size) : size(_size) {
        TOP = new Node();
        BOTTOM = new Node();
    }

    void insert(const int &ref) {
        if (!IS_FULL()) {
            // check ref is already exist in this stack
            Node* target = find(ref);
            if (target != TOP->down && target != BOTTOM) {
                // move target to top
                target->down->up = target->up;
                target->up->down = target->down;
                TOP->down->up = target;
                target->up = TOP;
            }
            else {
                Node *new_node = new Node(ref);
                new_node->up = TOP;
                new_node->down = target;
                target->up = new_node;
                ++size;
            }
        }
        else {
            // page_replacement
        }
    }
    Node* find(const int &ref) {
        Node* target = BOTTOM;
        for (int i = 0; i < size; ++i) {
            target = target->up;
            if (target->data == ref) {
                return target;
            }
        }
        return target; // not exist
    }
    bool IS_EMPTY() {
        return size == 0 ? true : false;
    }
    bool IS_FULL() {
        return size < 32 ? false : true;
    }

};

class TLB {
private:
    int size;
    int TLB_table[32][2]; // < page#, frame# >
public:
    TLB() : size(0) {
        memset(TLB_table, -1, sizeof(TLB_table));
    }
    void insert_page(const int &page_number, const int &frame_number) {
        if (!IS_FULL()) {
            
            ++size;
        }
        
    }
    void delete_page(const int &page_number) {
        
        --size;
    }
    
    int find_frame_by_page_number(const int &page_number) {
        for (int i = 0; i < 32; ++i) {
            if (TLB_table[i][0] == page_number)
                return TLB_table[i][1];
        }
        return -1; // page fault
    }
    
    bool IS_FULL() {
        return (this->size == 32) ? true : false;
    }
};

double EAT(const double &e, const double &a) {
    return 2 + e - a;
}

int main(int argc, const char * argv[]) {
    int page_table[256];
    int frame_table[256];
    memset(page_table, -1, sizeof(page_table));
    memset(frame_table, -1, sizeof(frame_table));
    // initialize page table
    ifstream logical_address_input("address.txt", ios::in);
    fstream physical_address_output("physical.txt", ios::out);
    int frame_number = 0;
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
            
            if (page_table[bin_to_dec(page_number)] == -1) {
                page_table[bin_to_dec(page_number)] = frame_number;
                frame_table[frame_number] = bin_to_dec(page_number);
                ++frame_number;
            }
            
            int physical_address = page_table[bin_to_dec(page_number)] * 256 + bin_to_dec(offset);
            physical_address_output << to_string(physical_address) << "\n";
        }
    }
    
    // create frame_table.txt
    ofstream frame_table_output("frame_table.txt", ios::out);
    // < frame#, flag, page# > free : flag(0), allocated : flag(1)
    for (int i = 0; i < 256; ++i) {
        if (frame_table[i] != -1)
            frame_table_output << to_string(i) << " " << "1" << " " << to_string(frame_table[i]) << "\n";
        else
            frame_table_output << to_string(i) << " " << "0" << " " << to_string(frame_table[i]) << "\n";
    }
    
    // LRU
    
    
    frame_table_output.close();
    physical_address_output.close();
    logical_address_input.close();
    
    return 0;
}

//class Node
//{
//public:
//    int height;
//    int key;
//    int value;
//    Node *parent, *left, *right;
//    Node() {                    // 생성자1
//        key = value = height = 0;
//        parent = left = right = NULL;
//    };
//    Node(int key, int value) {    // 생성자2
//        this->key = key;
//        this->value = value;
//        parent = left = right = NULL;
//    };
//};
//
//class AVLTree
//{
//public:
//    Node *root;
//    int n;
//    AVLTree() {        // 생성자
//        this->root = NULL;
//        n = 0;
//    };
//    Node* find( int key, Node* v);
//    Node* insert( int key,  int value);
//
//    int height(Node* v);
//    void setHeight(Node* v);
//    bool isBalanced( Node *v);
//    Node* tallGrandChild( Node* v);
//    Node* restructure(Node* v);
//    void rebalance(Node* v);
//    bool isExternal(Node* v);
//    Node* rotation_RR(Node* v);
//    Node* rotation_LL(Node* v);
//    Node* rotation_LR(Node* v);
//    Node* rotation_RL(Node* v);
//
//    int size();
//    bool empty();
//};
//// AVL 트리의 size 반환
//int AVLTree::size() {
//    return n;
//}
//
//// AVL 트리의 empty 여부 반환
//bool AVLTree::empty() {
//    return (n == 0);
//}
//
//// 노드v가 leaf 노드인지 확인
//bool AVLTree::isExternal(Node* v) {
//    return (v->left == NULL && v->right == NULL);
//}
//
//// 노드 v의 높이 반환
//int AVLTree::height(Node* v) {
//    if (v == NULL)
//        return -1;
//    else
//        return (isExternal(v)) ? 0 : v->height;
//}
//
//// 노드 v의 높이 계산
//void AVLTree::setHeight(Node* v) {
//    int hl = height(v->left);    // height of left child
//    int hr = height(v->right);    // height of right child
//    v->height = 1 + _max(hl, hr);
//}
//
////  노드 v에 대해 균형 여부 확인
//bool AVLTree::isBalanced( Node *v) {
//    int bal = height(v->left) - height(v->right);
//    return ((-1 <= bal) && (bal <= 1));
//}
//
//// 최초의 불균형 노드 z에 대해, height가 더 큰, z의 grandchld 탐색
//Node* AVLTree::tallGrandChild( Node* z) {
//    Node* zl = z->left;
//    Node* zr = z->right;
//    if (height(zl) >= height(zr)) {
//        if (height(zl->left) >= height(zl->right))    // 자식과 같은 방향의 손자를 우선적으로 선택
//            return zl->left;
//        else
//            return zl->right;
//    }
//    else {
//        if (height(zr->right) >= height(zr->left))    // 자식과 같은 방향의 손자를 우선적으로 선택
//            return zr->right;
//        else
//            return zr->left;
//    }
//}
//
//
//
///****************************************************************************************************************************************///학생이 구현.
//Node* AVLTree::rotation_RR(Node* par) {
//    Node* chi = par->right;
//    par->right = chi->left;
//    chi->left = par;
//    return chi;
//}
//Node* AVLTree::rotation_LL(Node* par) {
//    Node* chi = par->left;
//    par->left = chi->right;
//    chi->right = par;
//    return chi;
//}
//Node* AVLTree::rotation_RL(Node* par) {
//    Node* chi = par->right;
//    par->right = rotation_LL(chi);
//    return rotation_RR(par);
//}
//Node* AVLTree::rotation_LR(Node* par) {
//    Node* chi = par->left;
//    par->left = rotation_RR(chi);
//    return rotation_LL(par);
//}
//Node* AVLTree::restructure(Node* x) {
//    if (x->parent == NULL || x->parent->parent == NULL)
//        return NULL;
//    Node* y = x->parent;
//    Node* z = y->parent;
//    if (y->left == x) {
//        if (z->left == y) {
//            if (z->parent != NULL) {
//                if (z->parent->left == z)
//                    z->parent->left = y;
//                else
//                    z->parent->right = y;
//            }
//            y->parent = z->parent;
//            z->parent = y;
//            z->left = y->right;
//            if (y->right != NULL)
//                y->right->parent = z;
//            y->right = z;
//            return y;
//        }
//        else {
//            if (z->parent != NULL) {
//                if (z->parent->left == z)
//                    z->parent->left = x;
//                else
//                    z->parent->right = x;
//            }
//            x->parent = z->parent;
//            z->parent = y->parent = x;
//            z->right = x->left;
//            if (x->left != NULL)
//                x->left->parent = z;
//            y->left = x->right;
//            if (x->right != NULL)
//                x->right->parent = y;
//            x->left = z;
//            x->right = y;
//            return x;
//        }
//    }
//    else {
//        if (z->right == y) {
//            if (z->parent != NULL) {
//                if (z->parent->right == z)
//                    z->parent->right = y;
//                else
//                    z->parent->left = y;
//            }
//            y->parent = z->parent;
//            z->parent = y;
//            z->right = y->left;
//            if (y->left != NULL)
//                y->left->parent = z;
//            y->left = z;
//            return y;
//        }
//        else {
//            if (z->parent != NULL) {
//                if (z->parent->right == z)
//                    z->parent->right = x;
//                else
//                    z->parent->left = x;
//            }
//            x->parent = z->parent;
//            z->parent = y->parent = x;
//            z->left = x->right;
//            if (x->right != NULL)
//                x->right->parent = z;
//            y->right = x->left;
//            if (x->left != NULL)
//                x->left->parent = y;
//            x->right = z;
//            x->left = y;
//            return x;
//        }
//    }
//}
//
///*****************************************************************************************************************************************///학생이 구현
//
//
//
//
//
//// 노드 v부터 root 노드까지의 path 상에 노드의 height를 재설정하고 균형여부 확인
//// 불균형 노드가 있으면, restructuring
//void AVLTree::rebalance(Node* v) {
//    Node *z = v;
//    while (z->parent != NULL) {
//        z = z->parent;
//        setHeight(z);
//        if (!isBalanced(z)) {
//            Node* x = tallGrandChild(z);
//            z = restructure(x);        // -------------------> restructure() 를 직접 구현해야합니다.!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
//            setHeight(z->left);
//            setHeight(z->right);
//            setHeight(z);
//        }
//    }
//    root = z;
//}
//
//
//
////key가 삽입될 위치 반환
//Node* AVLTree::find(int key, Node* v) {
//    Node* leafNode;
//    leafNode = NULL;
//
//    while (v != NULL) {
//        leafNode = v;
//        if (key < v->key)
//            v = v->left;
//        else if (key > v->key)
//            v = v->right;
//        else
//            return v;
//    }
//
//    // leaf까지 도달한 경우, leaf노드를 반환한다.
//    return leafNode;
//}
//
//
////노드 삽입
//Node* AVLTree::insert(int key,  int value) {
//    Node *curLeafNode = find(key, root);
//    Node *newNode = new Node(key, value);
//    Node *insertNode = new Node();
//
//    if (key < curLeafNode->key)    // 삽입하려는 key가 현재 leaf노드의 key보다 작은 경우
//    {
//        curLeafNode->left = newNode;
//    }
//    else     // 삽입하려는 key가  현재 leaf노드의 key보다 큰 경우
//    {
//        curLeafNode->right = newNode;
//    }
//    newNode->parent = curLeafNode;
//    n++;
//
//    // restructure 및 height 갱신
//    setHeight(newNode);
//    rebalance(newNode);
//
//
//    // 삽입된 AVL tree의 노드의 height 출력
//    insertNode = find(key, root);
//    cout << insertNode->value << " " << insertNode->height << endl;
//    return insertNode;
//}
