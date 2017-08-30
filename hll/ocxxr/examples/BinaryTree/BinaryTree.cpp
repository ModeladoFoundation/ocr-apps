/*
 * BinaryTree.cpp
 *
 * A simple tree-based mapping data structure.
 */

#include <ocxxr-main.hpp>

#include <cstdlib>

constexpr bool kVerboseMessages = false;

#ifndef BT_PTR_TYPE
#define BT_PTR_TYPE RelPtr
#endif

template <typename T>
using Ptr = ocxxr::BT_PTR_TYPE<T>;

template <typename K, typename V>
class BinaryTree {
 public:
    static constexpr size_t kTreeSize =
            (1 << 24);  // Allocate enough space for 16M entries

    static ocxxr::Arena<BinaryTree> Create() {
        constexpr size_t bytes = sizeof(BinaryTree) + sizeof(Node) * kTreeSize;
        auto arena = ocxxr::Arena<BinaryTree>::Create(bytes);
        CreateIn(arena.Untyped());
        return arena;
    }

    static BinaryTree *CreateIn(ocxxr::Arena<void> arena) {
        return arena.New<BinaryTree>(arena);
    }

    BinaryTree(ocxxr::Arena<void> arena) : arena_(arena), root_(nullptr) {}

    bool Put(const K &key, const V &value) {
        if (kVerboseMessages) {
            PRINTF("Starting put op...\n");
        }
        return Update(key, value, root_);
    }

    bool Get(const K &key, V &result) {
        if (kVerboseMessages) {
            PRINTF("Starting get op...\n");
        }
        return Find(key, root_, result);
    }

    // TODO - add remove op, and use a freelist internally for nodes

 private:
    struct Node {
        const K key;
        V value;
        Ptr<Node> left, right;
        Node(const K &k, const V &v)
                : key(k), value(v), left(nullptr), right(nullptr) {}
    };

    ocxxr::Arena<void> arena_;
    Ptr<Node> root_;

    bool Update(const K &key, const V &value, Ptr<Node> &node) {
        if (!node) {
            // Base case 1: not found
            node = arena_.New<Node>(key, value);
            assert(node && "Arena out of memory");
            return false;
        } else if (node->key == key) {
            // Base case 2: found
            node->value = value;
            return true;
        } else {
            // Recursive case
            Ptr<Node> &next = (node->key < key) ? node->left : node->right;
            return Update(key, value, next);
        }
    };

    bool Find(const K &key, Node *node, V &output) {
        if (!node) {
            // Base case 1: not found
            return false;
        } else if (node->key == key) {
            // Base case 2: found
            output = node->value;
            return true;
        } else {
            // Recursive case
            Ptr<Node> &next = (node->key < key) ? node->left : node->right;
            return Find(key, next, output);
        }
    };
};

u64 myhash(u64 x) { return x * 11400714819323198549UL; }

void ocxxr::Main(ocxxr::Datablock<ocxxr::MainTaskArgs>) {

    auto tree = BinaryTree<u64, char>::Create();

    //
    // Puts
    //

    constexpr u64 kPutCount = 10'000'000;

    if (kVerboseMessages) {
        PRINTF("Starting puts\n");
    }

    for (u64 i = 0; i < kPutCount; i++) {
        tree->Put(myhash(i), static_cast<char>('a' + i % 26));
    }

    if (kVerboseMessages) {
        PRINTF("Done with puts\n");
    }

    //
    // Gets
    //

    if (kVerboseMessages) {
        PRINTF("Starting gets\n");
    }

    char result[] = {'X', '\0'};
    // Compute an index that should give us the value 'g'
    constexpr u64 index = 26 * (kPutCount / 32) + 6;
    tree->Get(myhash(index), result[0]);

    if (kVerboseMessages) {
        PRINTF("Done with gets\n");
    }

    //
    // Results
    //

    PRINTF("Result = %s\n", result);
    assert(result[0] == 'g');
    ocxxr::Shutdown();
}
