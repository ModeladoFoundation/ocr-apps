/*
 * This file is subject to the license agreement located in the file LICENSE
 * and cannot be distributed without it. This notice cannot be
 * removed or modified.
 */

// print out the entries in the symbol tables.

#include "rose.h"
#include <stdio.h>
#include <string.h>

using namespace std;
using namespace SageBuilder;
using namespace SageInterface;


string CurrentPathName;
int table_count=0;


class visitorTraversal : public AstSimpleProcessing
{
  public:
    visitorTraversal();
    void atTraversalStart();
    virtual void visit(SgNode* node);
    virtual void atTraversalEnd(SgProject* project);

};

visitorTraversal::visitorTraversal()
{
}

void visitorTraversal::atTraversalStart()
{
    CurrentPathName.clear();
}


void visitorTraversal::visit(SgNode* node)
{
    if (CurrentPathName.empty())
    {
        CurrentPathName = node->get_file_info()->get_filenameString();

        Rose_STL_Container<SgNode*> scopesList = NodeQuery::querySubTree(getFirstGlobalScope(getProject()),
                                                                         V_SgScopeStatement);
        for (Rose_STL_Container<SgNode *>::iterator iter = scopesList.begin();
             iter != scopesList.end(); iter++)
        {
            SgScopeStatement* scope = isSgScopeStatement(*iter);
            ROSE_ASSERT(scope != NULL);

            if (isSgGlobal(scope))
                printf("\nGlobal:\n");
            else
                printf("\nLocal:\n");

            string name = scope->get_symbol_table()->get_name().getString();
            scope->print_symboltable(name.c_str());

            printf("\n\n");
        }
    }
}


void visitorTraversal::atTraversalEnd(SgProject* project)
{
    printf ("Traversal ends here. \n");
}


int main ( int argc, char* argv[] )
{

    SgProject* project = frontend(argc,argv);
    ROSE_ASSERT (project != NULL);

    // Build the traversal object
    visitorTraversal exampleTraversal;

    // Call the traversal function (member function of AstSimpleProcessing)
    // starting at the project node of the AST, using a preorder traversal.
    exampleTraversal.traverseInputFiles(project,preorder);

    return 0;
}
