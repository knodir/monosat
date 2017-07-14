/*

 The MIT License (MIT)

 Copyright (c) 2013-2017, Sam Bayless

 Permission is hereby granted, free of charge, to any person obtaining a copy of this software and
 associated documentation files (the "Software"), to deal in the Software without restriction,
 including without limitation the rights to use, copy, modify, merge, publish, distribute,
 sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is
 furnished to do so, subject to the following conditions:

 The above copyright notice and this permission notice shall be included in all copies or
 substantial portions of the Software.

 THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT
 NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM,
 DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT
 OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 *This is code for finding a (locally) minimal satisfying cover set, with respect to a particular set of variables to be minimized.
 *It is adapted from 2QBF_DPLL by Daijue Tang, from A Comparative Study of 2QBF Algorithms[1]  Darsh Ranjan, Daijue Tang, and Sharad Malik
 *
 *
 * [1] A Comparative Study of 2QBF Algorithms, by D. Ranjan,
 * D. Tang and S. Malik, Proceedings of the Seventh
 * International Conference on Theory and Applications of
 * Satisfiability Testing (SAT2004), Vancouver, BC, Canada,
 * May 2004.

 */

#ifndef COVER_H_
#define COVER_H_
#include "core/Solver.h"
#include "core/SolverTypes.h"
#include "mtl/Vec.h"
#include "mtl/Sort.h"

namespace Minisat{



class Cover{

vec<vec<Lit> > uncovered;
vec<int> coverCount;
vec<int> mVarsGreedyScore;
vec<vec<int> > mVarsInUncCls;
vec<Lit> potentialCoverLits;
vec<bool>in_cover;
vec<Var> subset;
struct cover_lt {
	const Cover & outer;
	bool operator () (Lit a, Lit b) const {return outer.mVarsGreedyScore[var(a)]>outer.mVarsGreedyScore[var(b)]; }
	cover_lt(const Cover & _outer):outer(_outer){}
};

vec<bool> include;
public:
/**
 * Whether or not to include the variable v in the cover (default: false).
 */
void excludeFromCover(Var v, bool exclude){
	include.growTo(v+1,true);
	include[v]=!exclude;
}

//Adapted from 2qbf_DPLL
void getCover(Solver &S, vec<Lit> & cover)
{
	include.growTo(S.nVars(),true);
	if(subset.size()==0){
		for(int i = 0;i<include.size();i++){
			if(include[i]){
				subset.push(i);
			}
		}
	}
	cover.clear();
	in_cover.clear();
	in_cover.growTo(S.nVars());


	//first pass: find any clauses where the _only_ true literal is a required lit. Add all such lits to the cover set.
	for(int i = 0;i< ((S.decisionLevel()==0) ? S.trail.size():S.trail_lim[0]);i++)
	{
		Lit l = S.trail[i];
		if(include[var(l)]){
			assert(!in_cover[var(l)]);

			assert(S.value(l)!=l_False);
			in_cover[var(l)]=true;
			cover.push(l);
		}
	}
	if(opt_fast_partial){
		//first find any forced literals and add them to the cover.
		//Disabling this because it is too expensive.
		/*for(int j = 0;j<subset.size();j++){

			Var v = subset[j];
			assert(include[v]);


			Lit p = mkLit(v);
			if(S.value(p)==l_False){
				p=~p;
			}
			assert(S.value(p)==l_True);

			vec<Solver::Watcher>&  ws  = S.watches[p];

			for (int k = 0;k<ws.size();k++){
				Solver::Watcher        *i = &ws[k];
				Lit blocker = i->blocker;
				if (blocker!=lit_Undef && S.value(blocker) == l_True && (!include[var(blocker)] || in_cover[var(blocker)] )){
					//this clause is covered
					continue;
				}

				// Make sure the false literal is data[1]:
				CRef     cr        = i->cref;
				Clause&  c         = S.ca[cr];

				assert(c.size()>1);
				Lit other_watcher=c[0];
				if (c[0] == ~p)
					other_watcher=c[1];

				assert (S.value(other_watcher) == l_True);//otherwise this would be a conflict.

				if(!include[var(other_watcher)]){

				}else if( in_cover[var(other_watcher)]){
					//this clause is already covered

				}else{
					in_cover[var(other_watcher)]=true;
					cover.push(other_watcher);
				}
			}

		}*/
		//now find any clauses that are still uncovered, and cover them.
		for(int j = 0;j<subset.size();j++){
			Var v = subset[j];
			assert(include[v]);
			if(in_cover[v])
				continue;

			Lit p = mkLit(v);
			if(S.value(p)==l_False){
				p=~p;
			}
			assert(S.value(p)==l_True);

			vec<Solver::Watcher>&  ws  = S.watches[~p];

			for (int k = 0;k<ws.size();k++){
				Solver::Watcher        *i = &ws[k];
				Lit blocker = i->blocker;
				if (blocker!=lit_Undef && S.value(blocker) == l_True && (!include[var(blocker)] || in_cover[var(blocker)] )){
					//this clause is covered
					continue;
				}

				// Make sure the false literal is data[1]:
				CRef     cr        = i->cref;
				Clause&  c         = S.ca[cr];

				assert(c.size()>1);
				Lit other_watcher=c[0];
				if (c[0] == p)
					other_watcher=c[1];

				if (S.value(other_watcher) == l_True){
					if(!include[var(other_watcher)] || in_cover[var(other_watcher)] ){
						//this clause is covered
						continue;
					}else{
						assert(!in_cover[v]);//arbitrarily pick this literal to cover this clause. (should we try to use a different heuristic here? for example, select the watcher that is watching more clauses?)
						in_cover[v]=true;
						cover.push(p);
						break;
					}
				}else{
					//this is the _only_ true literal in this clause, so it must cover the clause
					assert(!in_cover[v]);
					in_cover[v]=true;
					cover.push(p);
					break;
				}
			}

		}
		return;
	}

	potentialCoverLits.clear();

	coverCount.clear();
	mVarsGreedyScore.clear();
	uncovered.growTo(S.clauses.size()+1);

	mVarsGreedyScore.growTo(S.nVars());
	mVarsInUncCls.growTo(S.nVars());
//
	coverCount.growTo(S.clauses.size()+1);
	for(int i = 0;i<mVarsInUncCls.size();i++)
		mVarsInUncCls[i].clear();
	for(int i = 0;i<uncovered.size();i++)
		uncovered[i].clear();

	for(int i = 0;i<S.clauses.size();i++)
	{
		int lit1count = 0;
		Lit plit = lit_Undef;

		Clause & c = S.ca[ S.clauses[i]];
		bool satisfied = false;

		for(int q = 0;q<c.size();q++)
		{
			Lit l = c[q];

			satisfied |=(S.value(l)==l_True);


			if(S.value(l)!=l_False){
				if(!include[var(l)])
				{
					lit1count = 0;
					break;
				}else{
					plit = l;

					if(lit1count++){
						assert(lit1count>1);
						break;
					}
				}
			}
		}
		assert(satisfied);
		if (lit1count == 1) {
			coverCount[i]++;
			assert(plit != lit_Undef);
			assert(include[var(plit)]);
			if(!in_cover[var(plit)])
			{
				assert(S.value(plit)!=l_False);
				in_cover[var(plit)]=true;

				cover.push(plit);
			}
		}
	}

	 //second pass
	for(int i = 0;i<S.clauses.size();i++)
	{
		Clause & c = S.ca[S.clauses[i]];
		bool sat = false;
		for(int j = 0;j<c.size();j++)
			{
				Lit l = c[j];
				if(S.value(l)==l_True){
					if(!include[var(l)])
					{
						coverCount[i]++;//mark this clause covered

						uncovered[i].clear();
						sat=true;
						break;
					}else if(in_cover[var(l)]){
/*
#ifndef NDEBUG
						{
					bool found=false;
				for(int q=0;q<cover.size();q++)
				{
					if(cover[q]==l)
					{
						found=true;
						break;
					}
				}
				assert(found);
						}
#endif
*/
						coverCount[i]++;//mark this clause covered

						uncovered[i].clear();
						sat=true;
						break;
					}else{
						uncovered[i].push(l);//add this lit to the set of potential lits to cover this clause.
					}
				}
			}

			if (sat)
				assert(uncovered[i].size()==0);
			else {//the sat lits in this clause are all excluded vars
			//increase the greed score
				assert(uncovered[i].size()>=0);
				 assert(coverCount[i]==0);
				for(int j = 0;j<uncovered[i].size();j++)
				{
					Lit l = uncovered[i][j];
					assert(include[var(l)]);
					Var v = var(l);
					int scr = mVarsGreedyScore[var(l)];
					if(mVarsGreedyScore[var(l)]++ == 0)
					{
						assert(include[var(l)]);
						assert(!(in_cover[var(l)]));
						potentialCoverLits.push(l);
					}
					 mVarsInUncCls[var(l)].push(i);

				}
			}
	}

	sort(potentialCoverLits,cover_lt(*this)); //sort the lits that might be used for covering by the number of clauses each covers.


	 //third pass
	bool done = false;
	int scoreRank =0;
	int i = 0;
	while (true) {

		for( ;i<S.clauses.size();i++)
		{
			if (coverCount[i]==0)
				break;
		}
		if(i==S.clauses.size()) //all clauses are covered.
			break;

			assert(scoreRank<potentialCoverLits.size());
			Lit inVid = potentialCoverLits[scoreRank++];
			int score = mVarsGreedyScore[var(inVid)];

			bool allcovered =true;
			vec<int> & inUncCls = mVarsInUncCls[var(inVid)];
			for (int j = 0; j < inUncCls.size(); j++) {

				int clause = inUncCls[j];
				if (coverCount[clause] == 0){ //sth new can be covered
					allcovered= false;
					break;
				}
			}

			if(!allcovered)
			{//there are still clauses that can be covered by this lit
				//sth new is garanteed to be covered
				assert(S.value(inVid)==l_True);
				cover.push(inVid);
				done=false;
				for (int j = 0; j < mVarsInUncCls[var(inVid)].size(); j++) {
					int clause = mVarsInUncCls[var(inVid)][j];
					coverCount[clause]++;
				}
			}
		}


	//finally, remove any vars that we can

	/* start from the beginning of the set, see if all the clauses
	 * a particular var covers already covered by later clauses, if
	 * that is the case, the var can be deleted from the cover set,
	 * and the cls cover count of the clses it covers will decrement*/

	int numUnessen = 0;

	for (i = 0; i < cover.size(); i++) {
		Lit l = cover[i];
		if ( mVarsInUncCls[var(l)].size()!=0)
			break;
	}
	int j = i;
	if (!done){
		for (; i < cover.size(); i++) {
			Lit l = cover[i];
			vec<int> & vidCovered = mVarsInUncCls[var(l)];

			assert(vidCovered.size()!=0);
			int k;
			for (k = 0; k < vidCovered.size(); k++) {
				int clsid = vidCovered[k];
				int count = coverCount[clsid];
				assert(coverCount[clsid] >= 1);
				if (coverCount[clsid] == 1) //this vid is essential
						break;
			}

			if (k >= vidCovered.size()) {//this vid is unessential, can get rid of
				//++numUnessen;
				//cover[i] = lit_Undef;
				for (int k = 0; k < vidCovered.size(); k++) {
					int clsid = vidCovered[k];
					assert(coverCount[clsid] >= 2);
					-- coverCount[clsid];
				}
			}else{
				cover[j++]=cover[i];
			}
		}
		cover.shrink(i-j);
	}
/*
#ifndef NDEBUG
	for(int i = 0;i<cover.size();i++)
	{
		assert(include[var(cover[i])]);
		assert(S.value(cover[i])==l_True);
	}

	for(int i = 0;i<S.clauses.size();i++)
	{
		Clause & c = S.ca[S.clauses[i]];
		bool sat = false;
		for(int j=0;j<c.size();j++)
		{

			if(S.value(c[j])==l_True && !include[var(c[j])])
			{
				sat=true;
				break;
			}

		}
		if(!sat)
		{
			bool found = false;
			for(int j=0;j<cover.size();j++)
			{
				Lit cov = cover[j];

				for(int k=0;k<c.size();k++)
				{
					if(c[k]==cov)
					{
						found=true;
						break;
					}
				}
				if(found)
					break;
			}
			assert(found);
		}
	}


#endif
*/
}
};
};
#endif /* PARTIAL_H_ */