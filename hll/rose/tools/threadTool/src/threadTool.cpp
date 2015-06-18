/*
 * This file is subject to the license agreement located in the file LICENSE
 * and cannot be distributed without it. This notice cannot be
 * removed or modified.
 */

// Add__thread to all global variables.

#include "rose.h"
#include <stdio.h>
#include <string.h>

using namespace std;
using namespace SageBuilder;
using namespace SageInterface;



class visitorTraversal : public AstSimpleProcessing
{
  public:
    visitorTraversal();
    virtual void visit(SgNode* node);
    virtual void atTraversalEnd();

};

visitorTraversal::visitorTraversal()
{
}

void visitorTraversal::visit(SgNode* node)
{
    // Add __thread to all variable declarations by setting TLS.

    if (isSgVariableDeclaration(node) != NULL)
    {
        SgVariableDeclaration* variableDeclaration = isSgVariableDeclaration(node);
        ROSE_ASSERT(variableDeclaration != NULL);
        SgInitializedNamePtrList::const_iterator i = variableDeclaration->get_variables().begin();
        ROSE_ASSERT(i != variableDeclaration->get_variables().end());

        do {
            SgInitializedName* variableName = *i;
            if (isSgGlobal(variableName->get_scope()))
            {
                // set TLS to add __thread
                variableDeclaration->get_declarationModifier().get_storageModifier().set_thread_local_storage(true);
            }
            i++;
        }
        while (i != variableDeclaration->get_variables().end());
    }
}


void visitorTraversal::atTraversalEnd()
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

    printf("Running all tests\n");
    // run all tests
    AstTests::runAllTests(project);

    printf("Calling backend\n");
    // Generate source code from AST and call the vendor's compiler
    return backend(project);
}
