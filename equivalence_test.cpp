/**********************************************************
* Ben Trombley 
*
* Testing for equivalence in forwarding tables.
* 
* Takes 3 arguments, the first two being forwarding tables
* with the following format: 128.153.48.0/20 2, and the 3rd
* being an output file, which will output "Yes" if they are
* equivalent, or the prefix range and differing hops for
* each range that they would not be equal.
**********************************************************/
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
// #include <stdio.h>
#include <stdlib.h>

using namespace std;

/* Global Variables */
bool found = false;
unsigned int power2[] =
    {1,2,4,8,16,32,64,128, 256, 512, 1024, 2048, 4096, 8192, 16384, 32768
    ,65536, 131072, 262144, 524288, 1048576, 2097152, 4194304, 8388608
    , 16777216, 33554432, 67108864, 134217728, 268435456, 536870912
    , 1073741824, 2147483648};

/* TrieNode
* representation of a node on a binary trie
* stores a hashchk value (to determine where on trie it is),
* match (tells if this node matches an inserted value), and
* nexthop, the value of the ip address with the prefix range
* up to that point of the trie.
*/
class TrieNode
{
private:
    unsigned int hashchk;
    bool match;
    int nexthop;
public:
    TrieNode *left, *right;

    TrieNode()
    {
        hashchk = nexthop = 0;
        left = right = NULL;
        match = false;
    }

    TrieNode(int hashval, bool yn)
    {
        hashchk = hashval;
        match = yn;
        left = right = NULL;
    }

    TrieNode(int hop) {
        nexthop = hop;
    }

    /* Accessors */
	unsigned int getHash() {return(hashchk);}
	bool getMatch() {return(match);}
	int getNexthop() {return(nexthop);}
	/* Mutators */
	void setMatch(bool yn) {match = yn;}
	void setNexthop(int next) {nexthop = next;}
};

/* True
* a class representation of a binary Trie
* at each matched node, the next hop value is stored
*/
class Trie
{
private:
    TrieNode *root;
public:
    Trie() { root = new TrieNode(power2[31],false); }

   	/* Insert ip address into trie */
    void insert(int p1, int p2, int p3, int p4, int len, int hop){
        TrieNode *current_node, *temp_node;
        int n = 32-len;
        int bit = 31;
        bool flag;
        unsigned int binary_val =
            p1*power2[24] + p2*power2[16] + p3*power2[8] + p4;
        if(!root) { root = new TrieNode(power2[31],false); }

        current_node = root;
        /* Step down the trie until reaching the prefix length */
        while (n <= bit) {
            flag = (power2[bit] & binary_val) ? true : false;

            /* Insert temporary nodes if necessary */
            if (flag && (!current_node->right)) {
                temp_node = new TrieNode(power2[bit-1],false);
                current_node->right = temp_node;
            }
            else if(!flag && (!current_node->left)){
                temp_node = new TrieNode(power2[bit-1],false);
                current_node->left = temp_node;
            }
            /* Move left if 0, right if 1 */
            current_node = (flag)? current_node->right:current_node->left;
            bit--;
        }
        current_node->setMatch(true);
        current_node->setNexthop(hop);
    }

    /* Search for next hop of specified ip. 
    *  No longer needed in this main program, but a useful function 
    *  For testing purposes
    */
    int search(unsigned binary_val)
    {
        int i, nexthop;
        bool flag;
        TrieNode *current_node;
        unsigned bit;
        current_node = root;
        while(current_node){
            if (current_node->getMatch()){
                nexthop = current_node->getNexthop();
            }
            bit = current_node->getHash();
            flag = (bit & binary_val) ? true : false;
            current_node = (flag)? current_node->right:current_node->left;
        }
        return nexthop;
    }

    TrieNode *getRoot(){
    	return root;
    }

};

/* Initialize the trie to contain the next hop info contained in filename
* returns true if file successfully opened, false if not
*/
bool init(Trie trie, string filename){
	string ipstring;
	string nexthopstring;
    char dummy;
    /* Open ifstream */
    ifstream ifs;    
    ifs.open(filename.c_str());
    if ( !ifs.is_open() ) {
      cout << "Could not open file " << filename << endl;
      return false;
    }
    int p1,p2,p3,p4,length,nexthop = 0;
    /* Read in ip address */
    while (!ifs.eof()){
        ifs >> ipstring;
        if(!ifs.eof()){
            ifs >> nexthopstring;
            istringstream iss(ipstring);
            /* Split the w.x.y.z/n into proper variables */
            iss >> p1 >> dummy >> p2 >> dummy >> p3 >> dummy
                >> p4 >> dummy >> length;

            nexthop = atoi(nexthopstring.c_str());
            /* Insert into the trie */
            trie.insert(p1,p2,p3,p4,length,nexthop);
        }
    }
    ifs.close();
    return true;
}

/* converts the given ip and hops into a string of the desired format */
string string_to_ip(string s, int len, int h1, int h2){
	s += string(32-s.length(),'0');
	string p1 = s.substr(0,8);
	string p2 = s.substr(8,8);
	string p3 = s.substr(16,8);
	string p4 = s.substr(24,8);
	int i1 = stoi(p1,0,2);
	int i2 = stoi(p2,0,2);
	int i3 = stoi(p3,0,2);
	int i4 = stoi(p4,0,2);
	return to_string(i1)+"."+to_string(i2)+"."+to_string(i3)+"."+to_string(i4)+"/"+to_string(len)
		+", "+to_string(h1)+", "+to_string(h2);
}

/* print_differences
*  traverses the tries until leaf nodes are reached, and prints the differences in hop values
*  t1 and t2 are pointers to the current nodes in the traversals, hop1 and hop2 are the
*  most recently encountered hop values during the traversal. s is the current ip string
*  in binary form, out is an output filestream for the printing.
*/
void print_differences (TrieNode * t1, int hop1, TrieNode * t2, int hop2, string s, ofstream &out){
	/* Once both leaf node hops have been determined, then if the hop
	*	values are different, print the prefix range and next hops
	*/
	if( t1 == NULL && t2 == NULL ) {	
		if(hop2 != hop1){
			out << string_to_ip(s,s.length(),hop1,hop2) << endl;
			found = true;
		}
		return;
	}
	
	/* If one trie is a leaf, then only continue down the other */
	else if( t1 == NULL) {
		if(t2->getMatch()){
			hop2 = t2->getNexthop();
		}
		print_differences(NULL,hop1,t2->left,hop2,s+"0",out);
		print_differences(NULL,hop1,t2->right,hop2,s+"1",out);
		return;
	}
	else if( t2 == NULL ) {
		if(t1->getMatch()){
			hop1 = t1->getNexthop();
		}
		print_differences(t1->left,hop1,NULL,hop2,s+"0",out);
		print_differences(t1->right,hop1,NULL,hop2,s+"1",out);
		return;
	}
	
	/* Set next hop values if one exists */
	if(t1->getMatch()) {hop1 = t1->getNexthop();}
	if(t2->getMatch()) {hop2 = t2->getNexthop();}

	/* Recurse both ways down the tree */
	string lstring = s+"0";
	string rstring = s+"1";
	print_differences(t1->left, hop1, t2->left, hop2, lstring, out);
	print_differences(t1->right, hop1, t2->right, hop2, rstring, out);
}

int main( int argc, char *argv[] ) {
	/* Verify a proper number of arguments */
	if (argc != 4) {
		cout << argv[0] << " requires three arguments (file1, file2, outputfile)" << endl;
	}
	else{
  		Trie trie1, trie2;
  		/* Ensure both tries were properly initialized */
  		if ( init(trie1,argv[1]) == false ){
  			cout << argv[1] << " could not be opened" << endl;
  			return 0;
  		}
    	if ( init(trie2,argv[2]) == false ){
	    	cout << argv[2] << " could not be opened" << endl;
  			return 0;	
    	}

    	string address = "";
    	/* Output filestresm */
    	ofstream ofs;
    	ofs.open(argv[3]);
    	/* Print the differences */
    	print_differences(trie1.getRoot(), 0, trie2.getRoot(), 0, address, ofs);
    	
    	/* If no differences were found, then print "Yes" */
    	if(!found) {
    		ofs << "Yes" << endl;
    	}
    	ofs.close();
	}
}
