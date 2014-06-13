
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
 * is implemented by:
 *<b>
 *<ul>
 *<li>tag_component
 *<li>aritm_expr
 *<li>aritm_term
 *<li>aritm_factor
 *</ul>
 *</b>
 */
public interface Itag_component
{
    public IToken getLeftIToken();
    public IToken getRightIToken();

    void accept(IAstVisitor v);
}


