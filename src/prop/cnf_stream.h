/*********************                                                        */
/** cnf_stream.h
 ** Original author: taking
 ** Major contributors: dejan
 ** Minor contributors (to current version): cconway, mdeters
 ** This file is part of the CVC4 prototype.
 ** Copyright (c) 2009, 2010  The Analysis of Computer Systems Group (ACSys)
 ** Courant Institute of Mathematical Sciences
 ** New York University
 ** See the file COPYING in the top-level source directory for licensing
 ** information.
 **
 ** This class takes a sequence of formulas.
 ** It outputs a stream of clauses that is propositionally
 ** equi-satisfiable with the conjunction of the formulas.
 ** This stream is maintained in an online fashion.
 **
 ** Unlike other parts of the system it is aware of the PropEngine's
 ** internals such as the representation and translation of [??? -Chris]
 **/

#include "cvc4_private.h"

#ifndef __CVC4__CNF_STREAM_H
#define __CVC4__CNF_STREAM_H

#include "expr/node.h"
#include "prop/sat.h"

#include <ext/hash_map>

namespace CVC4 {
namespace prop {

class PropEngine;

/**
 * Comments for the behavior of the whole class... [??? -Chris]
 * @author Tim King <taking@cs.nyu.edu>
 */
class CnfStream {
public:
  /** Cache of what nodes have been registered to a literal. */
  typedef __gnu_cxx::hash_map<SatLiteral, Node, SatSolver::SatLiteralHashFunction> NodeCache;

  /** Cache of what literals have been registered to a node. */
  typedef __gnu_cxx::hash_map<Node, SatLiteral, NodeHashFunction> TranslationCache;

private:

  /** The SAT solver we will be using */
  SatInputInterface *d_satSolver;

  TranslationCache d_translationCache;
  NodeCache d_nodeCache;

protected:

  /**
   * Asserts the given clause to the sat solver.
   * @param clause the clasue to assert
   */
  void assertClause(SatClause& clause);

  /**
   * Asserts the unit clause to the sat solver.
   * @param a the unit literal of the clause
   */
  void assertClause(SatLiteral a);

  /**
   * Asserts the binary clause to the sat solver.
   * @param a the first literal in the clause
   * @param b the second literal in the clause
   */
  void assertClause(SatLiteral a, SatLiteral b);

  /**
   * Asserts the ternary clause to the sat solver.
   * @param a the first literal in the clause
   * @param b the second literal in the clause
   * @param c the thirs literal in the clause
   */
  void assertClause(SatLiteral a, SatLiteral b, SatLiteral c);

  /**
   * Returns true if the node has been cashed in the translation cache.
   * @param node the node
   * @return true if the node has been cached
   */
  bool isCached(TNode node) const;

  /**
   * Returns the cashed literal corresponding to the given node.
   * @param node the node to lookup
   * @return returns the corresponding literal
   */
  SatLiteral lookupInCache(TNode n) const;

  /**
   * Caches the pair of the node and the literal corresponding to the
   * translation.
   * @param node node
   * @param lit
   */
  void cacheTranslation(TNode node, SatLiteral lit);

  /**
   * Acquires a new variable from the SAT solver to represent the node and
   * inserts the necessary data it into the mapping tables.
   * @param node a formula
   * @param theoryLiteral is this literal a theory literal (i.e. theory to be
   *        informed when set to true/false
   * @return the literal corresponding to the formula
   */
  SatLiteral newLiteral(TNode node, bool theoryLiteral = false);

public:

  /**
   * Constructs a CnfStream that sends constructs an equi-satisfiable set of clauses
   * and sends them to the given sat solver.
   * @param satSolver the sat solver to use
   */
  CnfStream(SatInputInterface* satSolver);

  /**
   * Destructs a CnfStream.  This implementation does nothing, but we
   * need a virtual destructor for safety in case subclasses have a
   * destructor.
   */
  virtual ~CnfStream() {
  }

  /**
   * Converts and asserts a formula.
   * @param node node to convert and assert
   * @param whether the sat solver can choose to remove this clause
   */
  virtual void convertAndAssert(TNode node) = 0;

  /**
   * Get the node that is represented by the given SatLiteral.
   * @param literal the literal from the sat solver
   * @return the actual node
   */
  Node getNode(const SatLiteral& literal);

  /**
   * Returns the literal that represents the given node in the SAT CNF
   * representation. [Presumably there are some constraints on the kind
   * of node? E.g., it needs to be a boolean? -Chris]
   *
   */
  SatLiteral getLiteral(TNode node);

  const TranslationCache& getTranslationCache() const;
  const NodeCache& getNodeCache() const;
}; /* class CnfStream */

/**
 * TseitinCnfStream is based on the following recursive algorithm
 * http://people.inf.ethz.ch/daniekro/classes/251-0247-00/f2007/readings/Tseitin70.pdf
 * The general idea is to introduce a new literal that
 * will be equivalent to each subexpression in the constructed equi-satisfiable
 * formula, then substitute the new literal for the formula, and so on,
 * recursively.
 * 
 * This implementation does this in a single recursive pass. [??? -Chris]
 */
class TseitinCnfStream : public CnfStream {

public:

  /**
   * Convert a given formula to CNF and assert it to the SAT solver.
   * @param node the formula to assert
   */
  void convertAndAssert(TNode node);

  /**
   * Constructs the stream to use the given sat solver.
   * @param satSolver the sat solver to use
   */
  TseitinCnfStream(SatInputInterface* satSolver);

private:

  // Each of these formulas handles takes care of a Node of each Kind.
  //
  // Each handleX(Node &n) is responsible for:
  //   - constructing a new literal, l (if necessary)
  //   - calling registerNode(n,l)
  //   - adding clauses assure that l is equivalent to the Node
  //   - calling toCNF on its children (if necessary)
  //   - returning l
  //
  // handleX( n ) can assume that n is not in d_translationCache
  SatLiteral handleAtom(TNode node);
  SatLiteral handleNot(TNode node);
  SatLiteral handleXor(TNode node);
  SatLiteral handleImplies(TNode node);
  SatLiteral handleIff(TNode node);
  SatLiteral handleIte(TNode node);
  SatLiteral handleAnd(TNode node);
  SatLiteral handleOr(TNode node);

  struct IteRewriteTag {};
  typedef expr::Attribute<IteRewriteTag, Node> IteRewriteAttr;
  Node handleNonAtomicNode(TNode node);

  /**
   * Transforms the node into CNF recursively.
   * @param node the formula to transform
   * @return the literal representing the root of the formula
   */
  SatLiteral toCNF(TNode node);

}; /* class TseitinCnfStream */

}/* prop namespace */
}/* CVC4 namespace */

#endif /* __CVC4__CNF_STREAM_H */
