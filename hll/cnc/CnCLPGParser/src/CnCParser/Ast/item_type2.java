
//
// This file is part of the CNC-C implementation and
// distributed under the Modified BSD License.
// See LICENSE for details.
//
// I AM A GENERATED FILE. PLEASE DO NOT CHANGE ME!!!
//

package CnCParser.Ast;

import lpg.runtime.*;

/**
 *<b>
 *<li>Rule 30:  item_type ::= T_STRUCT T_NAME
 *</b>
 */
public class item_type2 extends Ast implements Iitem_type
{
    public item_type2(IToken leftIToken, IToken rightIToken)
    {
        super(leftIToken, rightIToken);

        initialize();
    }

    public void accept(IAstVisitor v)
    {
        if (! v.preVisit(this)) return;
        enter((Visitor) v);
        v.postVisit(this);
    }

    public void enter(Visitor v)
    {
        v.visit(this);
        v.endVisit(this);
    }
}


