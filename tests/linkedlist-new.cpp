//
// Created by kiva on 2020/4/8.
//
#include <stdio.h>
#include <stdlib.h>

struct Node {
    int element;
    Node *next;
};

struct List {
    Node *head;
};

Node *new_Node(int value) {
    Node *node = new Node;
    node->element = value;
    node->next = nullptr;
    return node;
}

void delete_Node(Node *n) {
    delete n;
}

List *new_List() {
    List *list = new List;
    list->head = nullptr;
    return list;
}

void delete_List(List *list) {
    delete list;
}

// 增删查改

bool is_empty(List *list) {
    return list->head == nullptr;
}

void insert(List *list, int i) {
    // list->head == nullptr
    if (is_empty(list)) {
        list->head = new_Node(i);
        return;
    }

    Node *tail = nullptr;
    for (Node *c = list->head; c; c = c->next) {
        tail = c;
    }

    tail->next = new_Node(i);
}

void traverse(List *list) {
    for (Node *c = list->head; c; c = c->next) {
        printf("%d\n", c->element);
    }
}

int main() {
    List *l = new_List();
    insert(l, 1);
    insert(l, 2);
    insert(l, 3);
    insert(l, 4);
    traverse(l);
//    erase(l, 1);
}
