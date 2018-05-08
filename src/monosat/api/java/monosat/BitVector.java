/*
 The MIT License (MIT)

 Copyright (c) 2018, Sam Bayless

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
 */
package monosat;

import monosat.Lit;
import monosat.MonosatJNI;
import monosat.Solver;

import java.util.ArrayList;
import java.util.Collections;
import java.util.List;
import java.util.Objects;

public final class BitVector {
    protected final int id;
    private final int width;
    private final Solver solver;
    private final ArrayList<Lit> bits = new ArrayList<Lit>();

    /**
     * Creates a new BitVector using the given literals.
     * The width of the bitvector will equal literals.size().
     *
     * @param solver The solver that this bitvector will belong to.
     * @param literals A non-empty set of at most 64 literals that will back this bitvector, in LSB order:
     * literals.get(0) will represent the 1-bit of the bitvector, literals.get(1) the 2-bit, etc.
     */
    public BitVector(Solver solver, ArrayList<Lit> literals) {
        this.solver = solver;
        width = literals.size();
        if(width<=0){
            throw new IllegalArgumentException("BitVector must have a bit-width >= 0");
        }else if(width>64){
            throw new IllegalArgumentException("BitVector must have a bit-width <= 64");
        }
        id = MonosatJNI.newBitvector(solver.solverPtr, solver.bvPtr, solver.getLitBuffer(literals), literals.size());
        bits.addAll(literals);
        solver.registerBitVector(this);
    }

    /**
     * Creates a new BitVector of the specified width, with a constant value.
     * If introduceLiterals is true, then introduces width literals,
     * else creates a bitvector that has no literals associated with it.
     *
     * @param solver The solver that this bitvector will belong to.
     * @param width The number of bits in this BitVector.
     * Width must be a non-zero postiive integer <= 64.
     * @param constant A non-negative constant value that this BitVector will represent.
     * constant must be >=0, and < 2<<width.
     */
    public BitVector(Solver solver, int width, long constant) {
        this.solver = solver;
        if(width<=0){
            throw new IllegalArgumentException("BitVector must have a bit-width >= 0");
        }else if(width>64){
            throw new IllegalArgumentException("BitVector must have a bit-width <= 64");
        }
        if (constant < 0){
            throw new IllegalArgumentException("BitVectors can only represent values >=0");
        }
        if (constant >= (1L << width)){
            throw new IllegalArgumentException("BitVectors can only represent values <= (2^width-1)");
        }
        id = MonosatJNI.newBitvector_const(solver.solverPtr, solver.bvPtr, width, constant);
        this.width = width;
        for (int i = 0; i < width; i++) {
            if ((constant & (1 << i)) == 1) {
                bits.add(Lit.True);
            } else {
                bits.add(Lit.False);
            }
        }
        solver.registerBitVector(this);
    }

    /**
     * Creates a new BitVector of the specified width.
     * If introduceLiterals is true, then introduces width literals,
     * else creates a bitvector that has no literals associated with it.
     *
     * @param solver The solver that this bitvector will belong to.
     * @param width The number of bits in this BitVector.
     * Width must be a non-zero postiive integer <= 64.
     * @param introduceLiterals If true (the default), create width number of
     * new literals to represent the bitvector. If false, the no literals
     * are introduced for this bitvector.
     */
    public BitVector(Solver solver, int width, boolean introduceLiterals) {
        this.solver = solver;
        if (width < 0) {
            throw new IllegalArgumentException("BitVector width must be >=0");
        }else if(width>64){
            throw new IllegalArgumentException("BitVector must have a bit-width <= 64");
        }
        this.width = width;
        if (!introduceLiterals) {
            id = MonosatJNI.newBitvector_anon(solver.solverPtr, solver.bvPtr, width);
        } else {
            for (int i = 0; i < width; i++) {
                bits.add(new Lit(solver));
            }
            id = MonosatJNI.newBitvector(solver.solverPtr, solver.bvPtr, solver.getVarBuffer(bits, 0), bits.size());
        }
        solver.registerBitVector(this);
    }

    public BitVector(Solver solver, int width) {
        this(solver, width, true);
    }

    @Override
    public boolean equals(Object o) {
        if (this == o) return true;
        if (o == null || getClass() != o.getClass()) return false;
        BitVector bitVector = (BitVector) o;
        return id == bitVector.id &&
                Objects.equals(solver, bitVector.solver);
    }

    @Override
    public int hashCode() {
        return Objects.hash(id, solver);
    }

    /**
     * Get the solver that this BitVector belongs to.
     * @return the solver that this BitVector belongs to.
     */
    public Solver getSolver(){
        return solver;
    }

    /**
     * Return an immutable view of the literal that make up this bitvector
     * (if any). The returned list will either have exactly width literals,
     * or be empty.
     * @return the literals (if any) that make up this BitVector.
     */
    public List<Lit> getBits() {
        return Collections.unmodifiableList(bits);
    }

    /**
     * Get Lit bit from the bits backing this bitvector.
     * @param bit The index of the literal to retrieve.
     * @return The literal representing bit 'bit'.
     */
    public Lit get(int bit) {
        return bits.get(bit);
    }

    /**
     * Get the bitwidth of this (eg, number of bits) of this bitvector.
     * @return the bitwidth of this bitvector.
     */
    public int width() {
        return width;
    }

    /**
     * Get the bitwidth of this (eg, number of bits) of this bitvector.
     * @return the bitwidth of this bitvector.
     */
    public int size() {
        return width();
    }

    /**
     * Returns a literal which evaluates to true if this bitvector is greater than
     * compareTo, false otherwise.
     *
     * <blockquote><pre>{@code
     * BitVector a = new BitVector(4);
     * BitVector b = new BitVector(4);
     * Lit l = a.gt(b);
     * //Lit l will be true iff a > b, in the assignment chosen by the solver.
     * }</pre></blockquote>
     *
     * @param compareTo The BitVector that this BitVector will be compared to.
     * @return a literal that will evaluate to true iff the comparison holds.
     */
    public Lit gt(BitVector compareTo) {
        return compare(Comparison.GT,compareTo);
    }

    /**
     * Returns a literal which evaluates to true if this bitvector is greater or equal to
     * compareTo, false otherwise.
     *
     * <blockquote><pre>{@code
     * BitVector a = new BitVector(4);
     * BitVector b = new BitVector(4);
     * Lit l = a.geq(b);
     * //Lit l will be true iff a >= b, in the assignment chosen by the solver.
     * }</pre></blockquote>
     *
     * @param compareTo The BitVector that this BitVector will be compared to.
     * @return a literal that will evaluate to true iff the comparison holds.
     */
    public Lit geq(BitVector compareTo) {
        return compare(Comparison.GEQ,compareTo);
    }

    /**
     * Returns a literal which evaluates to true if this bitvector is less than
     * compareTo, false otherwise.
     *
     * <blockquote><pre>{@code
     * BitVector a = new BitVector(4);
     * BitVector b = new BitVector(4);
     * Lit l = a.lt(b);
     * //Lit l will be true iff a < b, in the assignment chosen by the solver.
     * }</pre></blockquote>
     *
     * @param compareTo The BitVector that this BitVector will be compared to.
     * @return a literal that will evaluate to true iff the comparison holds.
     */
    public Lit lt(BitVector compareTo) {
        return compare(Comparison.LT,compareTo);
    }

    /**
     * Returns a literal which evaluates to true if this bitvector is less or equal to
     * compareTo, false otherwise.
     *
     * <blockquote><pre>{@code
     * BitVector a = new BitVector(4);
     * BitVector b = new BitVector(4);
     * Lit l = a.leq(b);
     * //Lit l will be true iff a <= b, in the assignment chosen by the solver.
     * }</pre></blockquote>
     *
     * @param compareTo The BitVector that this BitVector will be compared to.
     * @return a literal that will evaluate to true iff the comparison holds.
     */
    public Lit leq(BitVector compareTo) {
        return compare(Comparison.LEQ,compareTo);
    }

    /**
     * Returns a literal which evaluates to true if this bitvector is not equal to
     * compareTo, false otherwise.
     *
     * <blockquote><pre>{@code
     * BitVector a = new BitVector(4);
     * BitVector b = new BitVector(4);
     * Lit l = a.neq(b);
     * //Lit l will be true iff a != b, in the assignment chosen by the solver.
     * }</pre></blockquote>
     *
     * @param compareTo The BitVector that this BitVector will be compared to.
     * @return a literal that will evaluate to true iff the comparison holds.
     */
    public Lit neq(BitVector compareTo) {
        return compare(Comparison.NEQ,compareTo);
    }

    /**
     * Returns a literal which evaluates to true if this bitvector is equal to
     * compareTo, false otherwise.
     *
     * <blockquote><pre>{@code
     * BitVector a = new BitVector(4);
     * BitVector b = new BitVector(4);
     * Lit l = a.eq(b);
     * //Lit l will be true iff a == b, in the assignment chosen by the solver.
     * }</pre></blockquote>
     *
     * @param compareTo The BitVector that this BitVector will be compared to.
     * @return a literal that will evaluate to true iff the comparison holds.
     */
    public Lit eq(BitVector compareTo) {
        return compare(Comparison.EQ,compareTo);
    }

    //Constant comparisons

    /**
     * Returns a literal which evaluates to true if this bitvector is greater than
     * compareTo, false otherwise.
     *
     * <blockquote><pre>{@code
     * BitVector a = new BitVector(4);
     * Lit l = a.gt(5);
     * //Lit l will be true iff a > 5, in the assignment chosen by the solver.
     * }</pre></blockquote>
     *
     * @param compareTo The non-negative long that this bitvector will be compared to.
     * compareTo must be < 2<<width().
     * @return a literal that will evaluate to true iff the comparison holds.
     */
    public Lit gt(long compareTo) {
        return compare(Comparison.GT,compareTo);
    }

    /**
     * Returns a literal which evaluates to true if this bitvector is greater or equal to
     * compareTo, false otherwise.
     *
     * <blockquote><pre>{@code
     * BitVector a = new BitVector(4);
     * Lit l = a.geq(5);
     * //Lit l will be true iff a >= 5, in the assignment chosen by the solver.
     * }</pre></blockquote>
     *
     * @param compareTo The non-negative long that this bitvector will be compared to.
     * compareTo must be < 2<<width().
     * @return a literal that will evaluate to true iff the comparison holds.
     */
    public Lit geq(long compareTo) {
        return compare(Comparison.GEQ,compareTo);
    }

    /**
     * Returns a literal which evaluates to true if this bitvector is less than
     * compareTo, false otherwise.
     *
     * <blockquote><pre>{@code
     * BitVector a = new BitVector(4);
     * Lit l = a.lt(5);
     * //Lit l will be true iff a < 5, in the assignment chosen by the solver.
     * }</pre></blockquote>
     *
     * @param compareTo The non-negative long that this bitvector will be compared to.
     * compareTo must be < 2<<width().
     * @return a literal that will evaluate to true iff the comparison holds.
     */
    public Lit lt(long compareTo) {
        return compare(Comparison.LT,compareTo);
    }

    /**
     * Returns a literal which evaluates to true if this bitvector is less or equal to
     * compareTo, false otherwise.
     *
     * <blockquote><pre>{@code
     * BitVector a = new BitVector(4);
     * Lit l = a.leq(5);
     * //Lit l will be true iff a <= 5, in the assignment chosen by the solver.
     * }</pre></blockquote>
     *
     * @param compareTo The non-negative long that this bitvector will be compared to.
     * compareTo must be < 2<<width().
     * @return a literal that will evaluate to true iff the comparison holds.
     */
    public Lit leq(long compareTo) {
        return compare(Comparison.LEQ,compareTo);
    }

    /**
     * Returns a literal which evaluates to true if this bitvector is not equal to
     * compareTo, false otherwise.
     *
     * <blockquote><pre>{@code
     * BitVector a = new BitVector(4);
     * Lit l = a.neq(5);
     * //Lit l will be true iff a != 5, in the assignment chosen by the solver.
     * }</pre></blockquote>
     *
     * @param compareTo The non-negative long that this bitvector will be compared to.
     * compareTo must be < 2<<width().
     * @return a literal that will evaluate to true iff the comparison holds.
     */
    public Lit neq(long compareTo) {
        return compare(Comparison.NEQ,compareTo);
    }

    /**
     * Returns a literal which evaluates to true if this bitvector is equal to
     * compareTo, false otherwise.
     *
     * <blockquote><pre>{@code
     * BitVector a = new BitVector(4);
     * Lit l = a.eq(5);
     * //Lit l will be true iff a == 5, in the assignment chosen by the solver.
     * }</pre></blockquote>
     *
     * @param compareTo The non-negative long that this bitvector will be compared to.
     * compareTo must be < 2<<width().
     * @return a literal that will evaluate to true iff the comparison holds.
     */
    public Lit eq(long compareTo) {
        return compare(Comparison.EQ,compareTo);
    }

    /**
     * Compare this bitvector to the bitvector 'compareTo'.
     * Returns a literal that will evaluate to true iff the comparison holds.
     *
     * <blockquote><pre>{@code
     * BitVector a = new BitVector(4);
     * BitVector b = new BitVector(4);
     * Lit l = a.compare(Comparison.LT,b);
     * //Lit l will be true iff a < b, in the assignment chosen by the solver.
     * }</pre></blockquote>
     *
     * See also the short forms: BitVector.gt(),geq(),lt(),leq(),eq(),neq().
     *
     * @param c type of comparison to perform (GEQ,EQ,LT, etc.)
     * @param compareTo The BitVector that this BitVector will be compared to.
     * @return a literal that will evaluate to true iff the comparison holds.
     */
    public Lit compare(Comparison c,BitVector compareTo){
        switch(c){
            case GT:
                return solver.toLit(MonosatJNI.newBVComparison_bv_gt(solver.solverPtr, solver.bvPtr, this.id, compareTo.id));
            case GEQ:
                return solver.toLit(MonosatJNI.newBVComparison_bv_geq(solver.solverPtr, solver.bvPtr, this.id, compareTo.id));
            case LT:
                return solver.toLit(MonosatJNI.newBVComparison_bv_lt(solver.solverPtr, solver.bvPtr, this.id, compareTo.id));
            case LEQ:
                return solver.toLit(MonosatJNI.newBVComparison_bv_leq(solver.solverPtr, solver.bvPtr, this.id, compareTo.id));
            case EQ:
                return solver.toLit(MonosatJNI.newBVComparison_bv_eq(solver.solverPtr, solver.bvPtr, this.id, compareTo.id));
            case NEQ:
                return solver.toLit(MonosatJNI.newBVComparison_bv_neq(solver.solverPtr, solver.bvPtr, this.id, compareTo.id));
        }
        //Comparison c should never be null
        throw new NullPointerException();
    }


    /**
     * Compare this bitvector to the bitvector 'compareTo'.
     * Returns a literal that will evaluate to true iff the comparison holds.
     *
     * <blockquote><pre>{@code
     * BitVector a = new BitVector(4);
     * Lit l = a.compare(Comparison.EQ,5);
     * //Lit l will be true iff a == 5, in the assignment chosen by the solver.
     * }</pre></blockquote>
     *
     * See also the short forms: BitVector.gt(),geq(),lt(),leq(),eq(),neq().
     *
     * @param c type of comparison to perform (GEQ,EQ,LT, etc.)
     * @param compareTo The non-negative long that this bitvector will be compared to.
     * compareTo must be < 2<<width().
     * @return a literal that will evaluate to true iff the comparison holds.
     */
    public Lit compare(Comparison c,long compareTo){
        switch(c){
            case GT:
                return solver.toLit(MonosatJNI.newBVComparison_const_gt(solver.solverPtr, solver.bvPtr, this.id, compareTo));
            case GEQ:
                return solver.toLit(MonosatJNI.newBVComparison_const_geq(solver.solverPtr, solver.bvPtr, this.id, compareTo));
            case LT:
                return solver.toLit(MonosatJNI.newBVComparison_const_lt(solver.solverPtr, solver.bvPtr, this.id, compareTo));
            case LEQ:
                return solver.toLit(MonosatJNI.newBVComparison_const_leq(solver.solverPtr, solver.bvPtr, this.id, compareTo));
            case EQ:
                return solver.toLit(MonosatJNI.newBVComparison_const_eq(solver.solverPtr, solver.bvPtr, this.id, compareTo));
            case NEQ:
                return solver.toLit(MonosatJNI.newBVComparison_const_neq(solver.solverPtr, solver.bvPtr, this.id, compareTo));
        }
        //Comparison c should never be null
        throw new NullPointerException();
    }

    /**
     * Creates a new bitvector consisting of the bits [this[0],..,this[size-1],append[0],..,append[append.size()-1]]
     * Does not introduce any new literals.
     *
     * @param append BitVector to concatenate to this one.
     * @return a new BitVector, consisting of the concatenation of this bitvector and 'append'
     */
    public BitVector concatenate(BitVector append) {
        int w = width() + append.width();
        BitVector result = new BitVector(solver, w);
        MonosatJNI.bv_concat(solver.solverPtr, solver.bvPtr, this.id, append.id, result.id);
        return result;
    }

    /**
     * Create a new bitvector consisting of the bits [this[lower],..,this[upper-1]]
     *
     * @param lower
     * @param upper
     * @return
     */
    public BitVector slice(int lower, int upper) {
        int w = upper - lower;
        assert (w >= 0);
        BitVector result = new BitVector(solver, w);
        MonosatJNI.bv_slice(solver.solverPtr, solver.bvPtr, this.id, lower, upper - 1, result.id);
        return result;
    }

    /**
     * Returns a Bitvector that represents the non-wrapping two's complement addition
     * of this and other. To prevent wrapping, the solver will enforce that a+b<(1<<width()).
     *
     * @param other
     * @return
     */
    public BitVector add(BitVector other) {
        return solver.add(this,other);
    }

    /**
     * Returns a Bitvector that represents the non-wrapping two's complement addition
     * of this and other. To prevent wrapping, the solver will enforce that a+b<(1<<width()).
     *
     * @param other
     * @return
     */
    public BitVector add(long other) {
        return solver.add(this,solver.bv(width(),other));
    }

    /**
     * Returns a Bitvector that represents the non-wrapping two's complement subtraction
     * of this and other. To prevent wrapping, the solver will enforce that a-b>=0.
     *
     * @param other
     * @return
     */
    public BitVector subtract(BitVector other) {
        return solver.subtract(this,other);
    }

    /**
     * Returns a Bitvector that represents the non-wrapping two's complement subtraction
     * of this and other. To prevent wrapping, the solver will enforce that a-b>=0.
     *
     * @param other
     * @return
     */
    public BitVector subtract(long other) {
        return solver.subtract(this,solver.bv(width(),other));
    }

    /**
     * Return the value of this bitvector from the solver.
     * Sometimes, a range of values may be determined by the solver to be satisfying.
     * If getMaximumValue is true, then largest value in that range will be returned,
     * otherwise, the smallest value is returned (this is relevant if optimization queries are being performed).
     *
     * @param getMaximumValue
     * @return
     */
    public long value(boolean getMaximumValue) {
        if (!MonosatJNI.hasModel(solver.solverPtr)) {
            throw new NoModelException("Solver has no model (this may indicate either that the solve() has not yet been called, or that the most recent call to solve() returned a value other than true, or that a constraint was added into the solver after the last call to solve()).");
        }
        return MonosatJNI.getModel_BV(solver.solverPtr, solver.bvPtr, id, getMaximumValue);
    }

    /**
     * Return the value of this bitvector from the solver.
     * Sometimes, a range of values may be determined by the solver to be satisfying.
     * In this case, the smallest value is returned (this is relevant if optimization queries are being performed).
     *
     * @return
     */
    public long value() {
        return this.value(false);
    }
}
