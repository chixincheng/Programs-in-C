#include <stdlib.h>
#include <stdio.h>

#include "bdd.h"
#include "debug.h"
#include "helper.h"

/*
 * Macros that take a pointer to a BDD node and obtain pointers to its left
 * and right child nodes, taking into account the fact that a node N at level l
 * also implicitly represents nodes at levels l' > l whose left and right children
 * are equal (to N).
 *
 * You might find it useful to define macros to do other commonly occurring things;
 * such as converting between BDD node pointers and indices in the BDD node table.
 */
#define LEFT(np, l) ((l) > (np)->level ? (np) : bdd_nodes + (np)->left)
#define RIGHT(np, l) ((l) > (np)->level ? (np) : bdd_nodes + (np)->right)

int indexnonleaf = BDD_NUM_LEAVES; //index of nonleaf node to be added, start at [256]
int firstent = -1; //helper var in bdd_from_raster
int newsize;//helper var in bdd_from_raster
int orgh;//helper var in bdd_from_raster

/**
 * Look up, in the node table, a BDD node having the specified level and children,
 * inserting a new node if a matching node does not already exist.
 * The returned value is the index of the existing node or of the newly inserted node.
 *
 * The function aborts if the arguments passed are out-of-bounds.
 */
int bdd_lookup(int level, int left, int right) {
    if(left == right){ //left child and right child are equal,no new node created
        return left;
    }
    else{
        BDD_NODE cmp = {level,left,right};
        int found = searchNode(level+left+right,cmp); //if found != -1, some existing node is found
        if(found != -1){ //a existing same node is found
            BDD_NODE *path = bdd_hash_map[found];//the pointer pointing to BDDNODE obj in bdd_nodes
            int sz = sizeof(BDD_NODE);//size of each BDD_NODE
            BDD_NODE *beg = &bdd_nodes[0];//beginning address of bdd_nodes[]
            int retindex = ((path - beg) / sz);//difference of address divide by the size of struct
            return retindex;
        }
        else{// new node is added
            int hkey = hashKey(level+left+right);
            int added = -1;
            bdd_nodes[indexnonleaf] = cmp; // add to bdd_nodes table
            int ret = indexnonleaf;
            // add to hashmap
            while(added == -1){
                if(bdd_hash_map[hkey] == NULL){
                    bdd_hash_map[hkey] = &bdd_nodes[indexnonleaf];
                    added = 0;
                }
                else{
                    hkey++;
                    if(hkey > BDD_HASH_SIZE-1){
                        hkey = hkey % BDD_HASH_SIZE;
                    }
                }
            }
            indexnonleaf++;
            return ret;
        }
    }
}

BDD_NODE *bdd_from_raster(int w, int h, unsigned char *raster) {
    if(firstent == -1){
        newsize = 2; //size of return matirx.
        orgh = h; //original height
        if(w == h){ //given square
            while(newsize < w){
                newsize = newsize * 2;
            }
        }
        else{ //given rectangle
            int cmp;
            if( w > h){
                cmp = w;
            }else{
                cmp = h;
            }
            while(newsize < cmp){
                newsize = newsize * 2;
            }
        }
        firstent = 0;
    }
    //loop through the new 2d array
    if(w == 1 && h == 1){ //base case

        return &bdd_nodes[*raster];//leave node can be access by direct indexing
    }
    else{
        if(firstent == 0){ //first time entering bddfromraster, expand the matrix
            w = newsize;
            h = newsize;
            firstent = -2;
        }
        int curw;
        int curh;
        if(w == h){ //divide horizontal
            curw = w;
            curh = h;
            w = w/2;
        }
        else{ //divide vertical
            curw = w;
            curh = h;
            h = h/2;
        }
        // the - &bdd_nodes[0] is to get the index of the node, since &bdd_ndoes[0] is the
        // starting index.
        int ret = bdd_lookup(levelcal(curw,curh),bdd_from_raster(w,h,rasterind(w,orgh,h,raster,1)) - &bdd_nodes[0]
            ,bdd_from_raster(w,h,rasterind(w,orgh,h,raster,2)) - &bdd_nodes[0]);
        return &bdd_nodes[ret];
    }
    return NULL;
}

void bdd_to_raster(BDD_NODE *node, int w, int h, unsigned char *raster) {
    // TO BE IMPLEMENTED
}

int bdd_serialize(BDD_NODE *node, FILE *out) {
    int serial = 0;//helper var
    BDD_NODE root = *node;
    int lev = root.level;
    unsigned char opc = lev+64;//opcode
    if(lev == 0){
        serial++;
        if(bdd_index_map[bdd_lookup(lev,root.left,root.right)] == 0){
            bdd_index_map[bdd_lookup(lev,root.left,root.right)] = serial;
            fputc(opc,out);
            //leave node's value = their index in bdd_nodes
            int leaveval = bdd_lookup(lev,root.left,root.right);
            fputc(leaveval,out);
            return serial;
        }
        else{
            return bdd_index_map[bdd_lookup(lev,root.left,root.right)];
        }
    }else{
        serial = bdd_serialize(&bdd_nodes[root.left],out) + 1;//traverse left
        if(bdd_index_map[bdd_lookup(lev,root.left,root.right)] == 0){
            bdd_index_map[bdd_lookup(lev,root.left,root.right)] = serial;
        }
        //
        serial = bdd_serialize(&bdd_nodes[root.right],out) + 1;//traverse right
        if(bdd_index_map[bdd_lookup(lev,root.left,root.right)] == 0){
            bdd_index_map[bdd_lookup(lev,root.left,root.right)] = serial;
        }
        fputc(opc,out);
        fputc(bdd_index_map[root.left],out);
        fputc(bdd_index_map[root.right],out);
    }

    return -1;
}

BDD_NODE *bdd_deserialize(FILE *in) {
    // TO BE IMPLEMENTED
    return NULL;
}

unsigned char bdd_apply(BDD_NODE *node, int r, int c) {
    // TO BE IMPLEMENTED
    return 0;
}

BDD_NODE *bdd_map(BDD_NODE *node, unsigned char (*func)(unsigned char)) {
    // TO BE IMPLEMENTED
    return NULL;
}

BDD_NODE *bdd_rotate(BDD_NODE *node, int level) {
    // TO BE IMPLEMENTED
    return NULL;
}

BDD_NODE *bdd_zoom(BDD_NODE *node, int level, int factor) {
    // TO BE IMPLEMENTED
    return NULL;
}
