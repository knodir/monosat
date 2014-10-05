/****************************************************************************************[Solver.h]
The MIT License (MIT)

Copyright (c) 2014, Sam Bayless

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
**************************************************************************************************/

#ifndef EDMONDS_KARP_DYNAMIC_H
#define EDMONDS_KARP_DYNAMIC_H

//dynamic edmonds_karp, implemented by Sam, following http://cstheory.stackexchange.com/questions/9938/incremental-maximum-flow-in-dynamic-graphs

#include "MaxFlow.h"
#include <vector>
#include "EdmondsKarp.h"
#include "EdmondsKarpAdj.h"
#include <algorithm>
namespace dgl{
template< class Capacity,typename Weight  >
class EdmondsKarpDynamic:public MaxFlow<Weight>{
	Weight f = 0;
	std::vector<Weight> F;

	Weight shortCircuitFlow=0;

	struct LocalEdge{
		int from;
		int id;
		bool backward=false;
		LocalEdge(int from=-1, int id=-1, bool backward=false):from(from),id(id),backward(backward){

			}
	};
	Weight curflow;
    int last_modification;
    int last_deletion;
    int last_addition;

    int history_qhead;
    int last_history_clear;
    std::vector<LocalEdge> prev;
    std::vector<Weight> M;
    DynamicGraph& g;
    Capacity & capacity;
    Weight INF;
#ifdef DEBUG_MAXFLOW
    	EdmondsKarpAdj<Capacity,Weight> ek;
#endif
    /*
     *            input:
               C, E, s, t, F
           output:
               M[t]          (Capacity of path found)
               P             (Parent table)
     */
    std::vector<int> Q;


    std::vector<bool> edge_enabled;

    Weight  BreadthFirstSearch(int s, int t,Weight bound=-1){
     	for(int i = 0;i<g.nodes();i++){//this has to go...
     		prev[i].from=-1;
     	}
     	prev[s].from = -2;
    	Q.clear();
           Q.push_back(s);
           bool found = false;
           Weight& old_m = M[s];
       	for(int j = 0;j<Q.size();j++){
   			   int u = Q[j];

               for (int i = 0;i<g.nIncident(u);i++){
            	   if(!edge_enabled[g.incident(u,i).id])
						continue;
            	   int id = g.incident(u,i).id;
            	   int v = g.incident(u,i).node;
                   ///(If there is available capacity, and v is not seen before in search)

            	   Weight& f = F[id];
            	   Weight c = capacity[id];

            	 //  int fr = F[id];
                   if (((c - F[id]) > 0) && (prev[v].from == -1)){
                       prev[v] = LocalEdge(u,id,false);
                       Weight b= c-F[id];
                       M[v] = std::min(M[u],b);
                       if (v != t)
                           Q.push_back(v);
                       else{
                    	   found=true;
                    	   break;
                       }

                           //return M[t];
                   }
               }
               for (int i = 0;i<g.nIncoming(u);i++){
				   int id = g.incoming(u,i).id;
				   if(!edge_enabled[(g.incoming(u,i).id)])
						continue;

				   int v = g.incoming(u,i).node;
					  ///(If there is available capacity, and v is not seen before in search)

				   Weight f = 0;
				   Weight c = F[id];

				 //  int fr = F[id];
					  if (((c - f) > 0) && (prev[v].from == -1)){
						  prev[v] = LocalEdge(u,id,true);

						  Weight b= c-f;
						  M[v] = std::min(M[u],b);
				/*		  */
						  if (v != t)
							  Q.push_back(v);
						  else{
	                    	   found=true;
	                    	   break;
	                       }
					  }
				  }
           }
       	M[s]=old_m;
       		if(found){
       			return M[t];
       		}
           return 0;


	   }


public:
    EdmondsKarpDynamic(DynamicGraph& _g,Capacity & cap):g(_g),capacity(cap),INF(0xF0F0F0)
#ifdef DEBUG_MAXFLOW
    	,ek(_g,cap)
#endif
    {
    	  curflow=0;
      	last_modification=-1;
      	last_deletion=-1;
      	last_addition=-1;

      	history_qhead=0;
      	last_history_clear=-1;
    	//setAllEdgeCapacities(1);
    }
    void setCapacity(int u, int w, Weight c){
    	//C.resize(g.edges());
    	//C[ ]=c;

    }
    void setAllEdgeCapacities(Weight c){

    }
    const  Weight maxFlow(int s, int t){
    	//see http://cstheory.stackexchange.com/a/10186
    	static int it = 0;
    	if(++it==56){
    		int a=1;
    	}
#ifdef RECORD
		if(g.outfile){
			fprintf(g.outfile,"f %d %d\n", s,t);
			fflush(g.outfile);
		}
#endif

    	//C.resize(g.nodes());
#ifdef DEBUG_MAXFLOW
    	for(int i = 0;i<g.all_edges.size();i++){
    		int id = g.all_edges[i].id;
    		Weight cap = capacity[id];
    		int from =  g.all_edges[i].from;
    		int to =  g.all_edges[i].to;

    		ek.setCapacity(from,to,cap);
    	}
#endif
      	if(last_modification>0 && g.modifications==last_modification){
#ifdef DEBUG_MAXFLOW
      		Weight expected_flow =ek.maxFlow(s,t);
#endif

#ifdef DEBUG_MAXFLOW
      		assert(curflow==expected_flow);
#endif
        	return curflow;
        }else if (last_modification<=0 || g.historyclears!=last_history_clear  || g.changed()){
        	F.clear();
        	F.resize(g.all_edges.size());
			prev.resize(g.nodes());
			M.resize(g.nodes());
			f=0;
			for(int i = 0;i<g.nodes();i++){
				prev[i].from =-1;
				M[i]=0;
			}
			prev[s].from = -2;
			 M[s] = INF;
			 edge_enabled.resize(g.edges());
			 for(int i = 0;i<g.edges();i++)
				 edge_enabled[i]= g.isEdge(i) && g.edgeEnabled(i);

			 dbg_print_graph(s,t,-1, -1);
			 f = maxFlow_p(s,t);
			 dbg_print_graph(s,t,-1, -1);
        }

#ifdef DEBUG_MAXFLOW
    	for(int i = 0;i<g.edges();i++){
			if(edge_enabled[i]){
				Weight fi = F[i];
				assert(F[i]<= capacity[i]);
			}else{
				assert(F[i]==0);
			}
		}
#endif
    	bool added_Edges=false;
    	bool needsReflow = false;

    	for (int i = history_qhead;i<g.history.size();i++){
    			int edgeid = g.history[i].id;
    			if(g.history[i].addition && g.edgeEnabled(edgeid)){
    				added_Edges=true;
    				edge_enabled[edgeid]=true;
    			}else if (!g.history[i].addition &&  !g.edgeEnabled(edgeid)){
    				//assert(edge_enabled[edgeid]);
    				edge_enabled[edgeid]=false;
    				Weight fv = F[edgeid]; //g.all_edges[edgeid].from;
    				if(fv==0){
    					//do nothing.
    				}else{
    					//ok, check if the maxflow from u to v has not lowered now that we've removed this edge.
    					//if it hasn't, then we are still safe
    					int u = g.all_edges[edgeid].from;
    					int v = g.all_edges[edgeid].to;

    					if (fv<0){
    						std::swap(u,v);
    						fv = -fv;
    					}
    					assert(fv>0);
    					Weight flow = maxFlow_residual(u,v, fv);//fix this!
    					assert(flow<=fv);
    					if(flow==fv){
    						//then we are ok.
    					}else{
    						//the total flow in the network has to be decreased by delta.
    						Weight delta = fv- flow;
    						assert(delta>0);
    						needsReflow=true;
    						//temporarily connect s and t by an arc of infinite capacity and run maxflow algorithm again from vin to vout
    						flow = maxFlow_p(u,v,s,t,delta);
    						//f-=flow;//this doesn't work
    					}
    					F[edgeid]=0;
    				}
    			}
    		}

    		//recompute the s-t flow
    		if (needsReflow){
    			f = 0;
    			//for(auto edge:g.adjacency[s]){
    			for(int i = 0;i<g.nIncident(s);i++){
    				auto & edge = g.incident(s,i);
    				if (edge_enabled[edge.id]){
    					f+=F[edge.id];
    				}else{
    					assert(F[edge.id]==0);
    				}
    			}
#ifndef NDEBUG
    			for(int i = 0;i<g.nIncoming(s);i++){
    				auto & edge = g.incoming(s,i);
    				//There shouldn't be any backwards flow to s in a maximum flow
    				assert(F[edge.id]==0);
				}
#endif
    		}
    		dbg_check_flow(s,t);
    		if(added_Edges)
    			f = maxFlow_p(s,t);

#ifdef DEBUG_MAXFLOW
    		Weight expected_flow =ek.maxFlow(s,t);
#endif
   		dbg_print_graph(s,t,-1, -1);
#ifdef DEBUG_MAXFLOW
    	assert(f==expected_flow);
    	for(int i = 0;i<g.edges();i++){
			if(g.edgeEnabled(i)){
				Weight fi = F[i];
				assert(F[i]<= capacity[i]);
			}else{
				assert(F[i]==0);
			}
		}
#endif

#ifndef NDEBUG
    	assert(edge_enabled.size()==g.edges());
    	for(int i = 0;i<g.edges();i++)
    		assert(edge_enabled[i]==g.edgeEnabled(i));
    	dbg_check_flow(s,t);
#endif

    	curflow=f;
		last_modification=g.modifications;
		last_deletion = g.deletions;
		last_addition=g.additions;

		history_qhead=g.history.size();
		last_history_clear=g.historyclears;
        return f;
    }


private:

    Weight maxFlow_residual(int s, int t, Weight & bound){
/*
#ifndef NDEBUG
    	DynamicGraph d;
		d.addNodes(g.nodes());
		std::vector<Weight> weights;
		for(int i = 0;i<g.edges();i++){
			if(g.isEdge(i)){
				if(edge_enabled[i]){
					Weight r =capacity[i]-F[i];
					d.addEdge(g.all_edges[i].from,g.all_edges[i].to,g.all_edges[i].id);//,r);
					weights.push_back(r);
				}else  {
					d.addEdge(g.all_edges[i].from,g.all_edges[i].to,g.all_edges[i].id,0);
					weights.push_back(0);
					d.disableEdge(g.all_edges[i].id);
				}
			}
		}
		for(int i = 0;i<g.edges();i++){
			if(edge_enabled[i]){
				if(F[i]>0){
					d.addEdge(g.all_edges[i].to,g.all_edges[i].from,-1);//,F[i]);
					weights.push_back(-1);
				}
			}
		}
#endif
*/
		Weight new_flow = 0;
        	 while(true){
        		 dbg_print_graph(s,t,-1, -1);
        		 Weight m= BreadthFirstSearch(s,t,bound);
    			if(bound >=0 && new_flow+m>bound){
					m=bound-new_flow;
				}

    			if (m <= 0)
    				break;

    			new_flow = new_flow + m;

    			int v = t;
    			while (v!=  s){
    				int u = prev[v].from;
    				int id = prev[v].id;
    				if(prev[v].backward){
    					F[id] = F[id] - m;
    				}else
    					F[id] = F[id] + m;
    			/*	if(rev[id]>-1){
    					F[rev[id]]-=m;
    				}*/
    			   // F[v][u] = F[v][u] - m;
    				assert(F[id]<= capacity[id]);
    				v = u;
    			}
    			dbg_print_graph(s,t,-1, -1);
    		}
/*
#ifndef NDEBUG
			EdmondsKarpAdj<std::vector<Weight>,Weight> ek_check(d,weights);

				Weight expect =  ek_check.maxFlow(s,t);
        		assert(new_flow<=expect);
    #endif
*/

        	 return new_flow;
        }

    Weight maxFlow_p(int s, int t){
    	dbg_print_graph(s,t,-1, -1);
    	 while(true){
    		 Weight m= BreadthFirstSearch(s,t);

			if (m == 0)
				break;

			f = f + m;

			int v = t;
			while (v!=  s){
				int u = prev[v].from;
				int id = prev[v].id;
				if(id==29){
					int a=1;
				}
				if(prev[v].backward){
					F[id] = F[id] - m;
			/*		if(rev[id]>-1){
						if(rev[id]==29){
							int a=1;
						}
						F[rev[id]]+=m;
					}*/
				}else{
					F[id] = F[id] + m;

				/*	if(rev[id]>-1){
						F[rev[id]]-=m;
					}*/
				}
				assert(F[id]<= capacity[id]);
			   // F[v][u] = F[v][u] - m;
				v = u;
			}

		}

#ifndef NDEBUG
    		//EdmondsKarp<EdgeStatus> ek_check(g);
    	 	 EdmondsKarpAdj<Capacity,Weight> ek_check(g,capacity);
    		Weight expect =  ek_check.maxFlow(s,t);
    		assert(f==expect);
#endif
    	 return f;
    }

    Weight  BreadthFirstSearch(int s, int t, int shortCircuitFrom, int shortCircuitTo, Weight& shortCircuitCapacity, Weight & shortCircuitFlow){
         	for(int i = 0;i<g.nodes();i++)//this has to go...
         		prev[i].from=-1;
         	prev[s].from = -2;
        	Q.clear();
            Q.push_back(s);

           	for(int j = 0;j<Q.size();j++){
       			   int u = Q[j];

       			   if(u==shortCircuitFrom){
       				   int v = shortCircuitTo;
       				   Weight f = shortCircuitFlow;
       				   Weight c = shortCircuitCapacity;
       				   if (((c - f) > 0)){
       					   prev[v] = {u,-1};
       					   Weight b = c-f;
						   M[v] = std::min(M[u], b);
						   if (v != t)
							   Q.push_back(v);
						   else
							   return M[t];
					   }
       			   }

                   for (int i = 0;i<g.nIncident(u);i++){
                	   if(!edge_enabled[g.incident(u,i).id])
    						continue;
                	   int id = g.incident(u,i).id;
                	   int v = g.incident(u,i).node;
                       ///(If there is available capacity, and v is not seen before in search)
                	   if(id==27 || id==29){
                							   int a=1;
                						   }
                	   Weight f = F[id];
                	   const Weight& c = capacity[id];

                	 //  int fr = F[id];
                       if (((c - f) > 0) && (prev[v].from == -1)){
                           prev[v] = LocalEdge(u,id,false);
                           Weight b = c-f;
                           M[v] = std::min(M[u], b);
                           if (v != t)
                               Q.push_back(v);
                           else
                               return M[t];
                       }
                   }

                   for (int i = 0;i<g.nIncoming(u);i++){
                	   int id = g.incoming(u,i).id;
					   if(!edge_enabled[(g.incoming(u,i).id)])
							continue;

					   int v = g.incoming(u,i).node;
						  ///(If there is available capacity, and v is not seen before in search)

					   Weight f = 0;
					   const Weight& c = F[id];

					 //  int fr = F[id];
						  if (((c - f) > 0) && (prev[v].from == -1)){
							  prev[v] = LocalEdge(u,id,true);
							  Weight b = c-f;
							  M[v] = std::min(M[u],b);
							  if (v != t)
								  Q.push_back(v);
							  else
								  return M[t];
						  }
					  }

               }
               return 0;


    	   }
    void dbg_print_graph(int from, int to, Weight shortCircuitFrom=-1, Weight shortCircuitTo=-1){
#ifndef NDEBUG
    	return;
    		static int it = 0;
    		if(++it==6){
    			int a =1;
    		}
    		printf("Graph %d\n", it);
    			printf("digraph{\n");
    			for(int i = 0;i<g.nodes();i++){
    				if(i==from){
    					printf("n%d [label=\"From\", style=filled, fillcolor=blue]\n", i);
    				}else if (i==to){
    					printf("n%d [label=\"To\", style=filled, fillcolor=red]\n", i);
    				}else
    					printf("n%d\n", i);
    			}

    			for(int i = 0;i<g.edges();i++){
    				if(edge_enabled[i]){
						auto & e = g.all_edges[i];
						const char * s = "black";
						std::cout<<"n" << e.from <<" -> n" << e.to << " [label=\"" << i <<": " <<  F[i]<<"/" << capacity[i]  << "\" color=\"" << s<<"\"]\n";
						//printf("n%d -> n%d [label=\"%d: %d/%d\",color=\"%s\"]\n", e.from,e.to, i, F[i],capacity[i] , s);
    				}
    			}

    			if(shortCircuitFrom>=0){
    				//printf("n%d -> n%d [label=\"%d: %d/%d\",color=\"black\"]\n", shortCircuitFrom,shortCircuitTo, -1, shortCircuitFlow,-1 );
    			}

    			printf("}\n");
#endif
    		}

    Weight maxFlow_p(int s, int t, int shortCircuitFrom, int shortCircuitTo, Weight & bound){
    	//
    	Weight newFlow = 0;
#ifndef NDEBUG
/*    	 	DynamicGraph d;
    	 	d.addNodes(g.nodes());
    	 	//std::vector<int> R;
    	 	for(int i = 0;i<g.edges();i++){
    	 		if(edge_enabled[i]){
    	 			Weight r =capacity[i]-F[i];
    	 			d.addEdge(g.all_edges[i].from,g.all_edges[i].to,g.all_edges[i].id);//,r);
    	 		}else if(g.isEdge(i)){
    	 			d.addEdge(g.all_edges[i].from,g.all_edges[i].to,g.all_edges[i].id,0);
    	 			d.disableEdge(g.all_edges[i].id);
    	 		}
    	 	}
    		for(int i = 0;i<g.edges();i++){
				if(edge_enabled[i]){
					if(F[i]>0){
						d.addEdge(g.all_edges[i].to,g.all_edges[i].from,-1,F[i]);
					}
				}
			}

    		int stflow = 0;
			//for(auto edge:g.inverted_adjacency[s]){
    		for(int i = 0;i<g.nIncoming(s);i++){
				auto & edge = g.incoming(s,i);
				if (edge_enabled[edge.id]){
					stflow+=F[edge.id];
				}
			}*/

#endif

    	shortCircuitFlow=0;
        	 while(true){
        		 dbg_print_graph(s,t,shortCircuitFrom, shortCircuitTo);
    			Weight m= BreadthFirstSearch(s,t,shortCircuitFrom,shortCircuitTo,bound,shortCircuitFlow);
    			if(bound >=0 && newFlow+m>bound){
						m=bound-newFlow;
					}
    			if (m == 0)
    				break;

    			newFlow = newFlow + m;

    			int v = t;
    			while (v!=  s){
    				int u = prev[v].from;
    				if(u==s){
    					int a=1;
    				}
    				int id = prev[v].id;
    				if(id>=0){
    					if(prev[v].backward){
    						F[id] = F[id] - m;
						/*	if(rev[id]>-1){
								F[rev[id]]+=m;
							}*/
    					}else{
							F[id] = F[id] + m;
					/*		if(rev[id]>-1){
								F[rev[id]]-=m;
							}*/
    					}
        				assert(id>=0);
        				assert(id<F.size());
        				assert(id<capacity.size());
        				assert(F[id]<= capacity[id]);
    				}else{
    					u = shortCircuitFrom;
    				}

    			   // F[v][u] = F[v][u] - m;
    				v = u;
    			}
    		}
#ifndef NDEBUG
        	 dbg_print_graph(s,t,shortCircuitFrom, shortCircuitTo);
    	 /*	if(shortCircuitFrom>=0){
    	 		d.addEdge(shortCircuitFrom,shortCircuitTo,d.edges(),100);
    	 	}
    		EdmondsKarpAdj<std::vector<Weight>,Weight> ek_check(d,capacity);

    		Weight expect =  ek_check.maxFlow(s,t);


    		assert(newFlow<=expect);

    		for(int i = 0;i<g.edges();i++){

					Weight fi = F[i];
					assert(F[i]<= capacity[i]);

    		}*/

#endif
        	 return newFlow;
        }

    void dbg_check_flow(int s, int t){
#ifndef NDEBUG
    	for(int u = 0;u<g.nodes();u++){
    		Weight inflow = 0;
    		Weight outflow = 0;
            for (int i = 0;i<g.nIncoming(u);i++){
				   int id = g.incoming(u,i).id;
				   assert(id<edge_enabled.size());
				   if(!edge_enabled[(g.incoming(u,i).id)])
						continue;

				   inflow+=F[id];
            }
            for (int i = 0;i<g.nIncident(u);i++){
				   int id = g.incident(u,i).id;
				   assert(id<edge_enabled.size());
				   if(!edge_enabled[(g.incident(u,i).id)])
						continue;

				   outflow+=F[id];
            }
            if(u!=s && u != t){
            	assert(inflow==outflow);
            }else if (u==s){
            	assert(outflow==f);
            }else if (u==t){
            	assert(inflow==f);
            }
    	}

#endif
    }

    std::vector<bool> seen;
    std::vector<bool> visited;
public:
    const Weight minCut(int s, int t, std::vector<MaxFlowEdge> & cut){
    	Weight f = maxFlow(s,t);
    	//ok, now find the cut
    	Q.clear();
    	Q.push_back(s);
    	seen.clear();
    	seen.resize(g.nodes());
    	seen[s]=true;
    	dbg_print_graph(s,t);
    	//explore the residual graph
    	for(int j = 0;j<Q.size();j++){
		   int u = Q[j];

    		for(int i = 0;i<g.nIncident(u);i++){
    			if(!g.edgeEnabled(g.incident(u,i).id))
    				continue;
    			int v = g.incident(u,i).node;
    			int id = g.incident(u,i).id;
    			if(capacity[id] - F[id] == 0){
    				cut.push_back(MaxFlowEdge{u,v,id});//potential element of the cut
    			}else if(!seen[v]){
    				Q.push_back(v);
    				seen[v]=true;
    			}
    		}
    		for(int i = 0;i<g.nIncoming(u);i++){
				if(!g.edgeEnabled(g.incoming(u,i).id))
					continue;
				int v = g.incoming(u,i).node;
				int id = g.incoming(u,i).id;
				if(F[id] == 0){

				}else if(!seen[v]){
					Q.push_back(v);
					seen[v]=true;
				}
			}
    	}
    	//Now keep only the edges from a seen vertex to an unseen vertex
    	int i, j = 0;
    	for( i = 0;i<cut.size();i++){
    		if(!seen[cut[i].v] && seen[cut[i].u]){
    			cut[j++]=cut[i];
    		}
    	}
    	cut.resize(j);
#ifndef NDEBUG
		Weight dbg_sum = 0;
		for(int i = 0;i<cut.size();i++){
			int id = cut[i].id;
			assert(F[id]==capacity[id]);
			dbg_sum+=F[id];
		}
		assert(dbg_sum==f);
#endif
    	return f;
    }
    const Weight getEdgeCapacity(int id){
     	assert(g.edgeEnabled(id));
     	return capacity[id];
     }
    const Weight getEdgeFlow(int id){
    	assert(g.edgeEnabled(id));
    	return F[id];// reserve(id);
    }
    const Weight getEdgeResidualCapacity(int id){
    	assert(g.edgeEnabled(id));
    	return  capacity[id]-F[id];// reserve(id);
    }
};
};
#endif

