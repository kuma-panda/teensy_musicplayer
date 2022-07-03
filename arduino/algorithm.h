#ifndef ALGORITHM_H
#define ALGORITHM_H

#include <arduino.h>

// -----------------------------------------------------------------------------
//  List
// -----------------------------------------------------------------------------
class ListNode
{
    public:
        void *value;
        ListNode *next;
        ListNode(void *value){
            this->value = value;
            this->next = nullptr;
        }
};

class List
{
    typedef bool (*ITERATE_PROC)(int, void *, void *);
    typedef bool (*PREDICT_PROC)(void *);
    private:
        ListNode *m_root;
        int       m_count;
    public:
        List() : m_root(nullptr), m_count(0){}
        void add(void *value){
            if( this->m_root == nullptr )
            {
                this->m_root = new ListNode(value);
                ++(this->m_count);
                return;
            }
            ListNode *node = this->m_root;
            while( node->next != nullptr )
            {
                node = node->next;
            }
            node->next = new ListNode(value);
            ++(this->m_count);
        }
        int getCount(){ return this->m_count; }
        bool forEach(ITERATE_PROC proc, void *param){
            ListNode *node = this->m_root;
            int i = 0;
            bool aborted = false;
            while( node != nullptr )
            {
                if( !proc(i, node->value, param) )
                {
                    aborted = true;
                    break;
                }
                node = node->next;
                i++;
            }
            return aborted;
        }
        void *operator[](int index){
            if( index >= this->m_count )
            {
                return nullptr;
            }
            ListNode *node = this->m_root;
            while( index > 0 )
            {
                node = node->next;
            }
            return node->value;
        }
        void *find(PREDICT_PROC proc){
            ListNode *node = this->m_root;
            while( node != nullptr )
            {
                if( proc(node->value) )
                {
                    return node->value;
                }
                node = node->next;
            }
            return nullptr;
        }
};

// -----------------------------------------------------------------------------
//  HashMap
// -----------------------------------------------------------------------------
class HashNode
{
    public:
        HashNode *next;
        uint16_t  key;
        void     *value;
        HashNode(uint16_t key, void *value){
            this->next = nullptr;
            this->key = key;
            this->value = value;
        }
};

class HashTable
{
    private:
        enum{MAX_NODE_NUM = 29};
        HashNode *m_table[MAX_NODE_NUM];

    public:
        HashTable(){
            for( int i = 0 ; i < HashTable::MAX_NODE_NUM ; i++ )
            {
                this->m_table[i] = nullptr;
            }
        }
        void add(uint16_t key, void *value){
            HashNode *node = this->m_table[key % HashTable::MAX_NODE_NUM];
            if( node == nullptr )
            {
                this->m_table[key % HashTable::MAX_NODE_NUM] = new HashNode(key, value);
                return;
            }
            while( node != nullptr )
            {
                if( node->key == key )
                {
                    node->value = value;
                    return;
                }
                if( node->next == nullptr )
                {
                    node->next = new HashNode(key, value);
                    return;
                }
                node = node->next;
            }
        }

        HashNode *get(uint16_t key){
            HashNode *node = this->m_table[key % HashTable::MAX_NODE_NUM];
            while( node != nullptr )
            {
                if( node->key == key )
                {
                    return node;
                }
                node = node->next;
            }
            return nullptr;
        }
};

#endif
