import numpy as np
import combinations as cs
import iofuncs as iof
import matplotlib.pyplot as plt

import matrix
import sorting
import monocheck
import maxslope
import leastnonmono

## @package findprogvar
# Package containing findC, a function which selects the best progress variable based on the given data files

## Determines the best progress variable for the given data files
#
# This function returns a matrix containing the following:
# (row1) stoich prog var (row2) file indices (row3) stoich Temps, sorted by row1
#
# This function also produces plots of the progress variables: output/CvsTemp.pdf.
# Note that a directory "output" must exist in the directory from which this function is called.
#
# The user can skip optimization by specifying options["SkipProgVar"] = 'yes'
#
# Note: this Python script relies on C++ functions connected through SWIG, which must generate the 
# following modules:
# - matrix
# - sorting
# - interpolator
# - monocheck
# - maxslope
# - least nonmono
#
# @param datafiles vector of strings specifying file names or paths for the desired data files.
# Note that the data files should be .kg files produced by FlameMaster
#
# @param testspecies vector of strings specifying the species to be considered in the progress variable
#
# @param bestC vector containing information about the best progress variable (output)
#
# @param options Python dictionary filled with user specified options for the program. Requires        
# options["sort method"], options["StoichMassFrac"], options["InterpMethod"], options["MaxSlopeTest"],
# options["PlotAllC"], options["lnmcheck"], and options["SkipProgVar"] to be specified

def findC(datafiles, testspecies, bestC, options): 

    # Interpolate each datafile, generate a matrix from interpolated data
    nofiles = len(datafiles)
    nocols = len(testspecies)+1
    locs = np.zeros(nocols-1)
    interpdata = np.zeros((nofiles,nocols))
    filesmatrix = np.zeros((nofiles,3))
    for ii in range(nofiles): # interpolate for each file
        dataobj = iof.ProcFile(datafiles[ii])
        if ii == 0:
            titles = dataobj.gettitles() 
        else:
            titles1 = dataobj.gettitles()
            np.testing.assert_array_equal(titles1,titles) # Verify that all data files have the same column headers
        dataobj.interpolate(testspecies, locs, interpdata[ii,:], interpval=float(options["StoichMassFrac"][0]), interpmethod="".join(options["InterpMethod"]))
    filesmatrix[:,0] = interpdata[:,0]
    filesmatrix[:,2] = interpdata[:,0] # filesmatrix stores (row1) stoich prog var (row2) file indices (row3) stoich Temps
    filesmatrix[:,1] = range(nofiles)
    filesmatC = matrix.Matrix(nofiles,3)
    for i in range(nofiles):
        for j in range(3):
            filesmatC.SetVal(i,j,filesmatrix[i,j])

    # Generate combinations matrix - skip this step if user input says to
    skip = options["SkipProgVar"][0]
    assert ((skip == 'yes') | (skip == 'no')), "must select input <yes> or <no> for <skip progress variable optimization:>"
    if skip == 'no':
        combosmatrix = np.zeros((nocols,cs.totnumcoms(nocols-1)+1))
        cs.combination_mat(combosmatrix[1:,1:])
    else:
        print "Skipping progress variable optimization, using user input"
        options["PlotAllC"][0] = 'no'
        combosmatrix = np.zeros((nocols,2))
        for i in range(nocols-1):
            combosmatrix[i+1,1] = 1
    combosmatrix[0,0] = 1

    # Calculate progress variables
    progvars = np.dot(interpdata,combosmatrix)
    length = progvars.shape[1]

    # Generate progress variable matrix
    progVar = matrix.Matrix(nofiles, length) 
    for i in range (nofiles):
        for j in range(length):
            progVar.SetVal(i,j,progvars[i,j])

    # Sort PROGVARS and FILESMATRIX by temperature
    sortmethod = options["sort method"][0]
    print "Sorting PROGVARS by temperature using %s sort" % sortmethod
    for i in [progVar, filesmatC]:
        if sortmethod  == 'bubble':
            sorter = sorting.BubbleSort(i)
        elif sortmethod == 'standard':
            sorter = sorting.StandardSort(i)
        elif sortmethod == 'brute':
            sorter = sorting.BruteSort(i)
        else:
            raise IOError("invalid sorting method (%s) specified, instead use <bubble> sort" % sortmethod)
        sorter.SetRefColNum(0)
        sorter.sort_data()
    print "Sorting FILESMATRIX by temperature"

    # Test monotonicity of PROGVARS
    print "Testing monotonicity \n"
    monoAry = np.zeros(length, dtype=np.int32)

    checker = monocheck.MonoCheck(progVar) # Create MonoCheck object
    assert checker.CheckStrictMonoticity(monoAry, 0) == 0, "CheckStrictMonoticity ran unsuccessfully.\n" 
    # ^ Check which columns of progVar are strictly increasing or strictly decreasing and store result in monoAry

    # Test for maximum slope if multiple monotonic progress variables are returned
    checksum = np.sum(monoAry)
    if checksum % 3 != 0:
        raise RuntimeError("Incorrect values in monoAry vector, check monotonicity function.\n")
    if checksum > 3:
        maxslopetest = options["MaxSlopeTest"][0]
        print "Testing max slope using %s" % maxslopetest
        if maxslopetest == 'linear regression':
            maxchecker = maxslope.LinRegression(progVar)
        elif maxslopetest == 'end point slope':
            maxchecker = maxslope.EndPointSlope(progVar)
        else:
            raise IOError("invalid maximum slope test (%s) specified, instead use <linear regression>" % maxslopetest)
        assert maxchecker.MostMonotonic(monoAry, 0) == 0, "MostMonotonic ran unsuccessfully.\n" 
        # ^ Distinguish the best monotonic progress variables
    elif checksum == 0:
        # Least non-monotonic tests to be implemented in beta version
        lnmoption = options["lnmcheck"][0]
        print "Finding least non-monotonic progress variable using %s nonmono check" % lnmoption
        if lnmoption == 'simple':
            lnm_check = leastnonmono.SimpleLNM(progVar)
        elif lnmoption == 'advanced':
            lnm_check = leastnonmono.AdvancedLNM(progVar)
        else:
            raise IOError("invalid lnm test (%s) specified, instead use <simple> or <advanced>" % maxslopetest)
        assert lnm_check.LeastNonMonotonic(monoAry, 0) == 0, "LeastNonMonotonic ran unsuccessfully.\n"

    # Print results
    monoAryflag = 0 
    for i in range(length): 
        if monoAry[i] == 3.0: # Print best monotonic progress variable if it exists
            if monoAryflag != 0:
                raise RuntimeError("Error in contents of monoAry vector: multiple best selected.\n")
            monoAryflag = 2
            bestC[:] = iof.get_progvar(combosmatrix[1:,i], testspecies, locs, i)
            print 'The chosen progress variable is C = %s' % bestC[1][0], 
            for j in bestC[1][1:]:
                print "+ %s" % j,
            print '\nThe column numbers of these species are ', bestC[0],', respectively.\n'
        elif monoAry[i] == 1.0: # Otherwise print least non-monotonic progress variable
            if monoAryflag != 0:
                raise RuntimeError("Error in contents of monoAry vector.\n")
            monoAryflag = 1
            bestC[:] = iof.get_progvar(combosmatrix[1:,i], testspecies, locs, i)
            print 'WARNING: no monotonic progress variables found, but proceeding with best alternative.\n'
            print 'The least non-monotonic progress variable is C = %s' % bestC[1][0], 
            for j in bestC[1][1:]: 
                print "+ %s" % j,
            print '\nThe column numbers of these species are', bestC[0],', respectively.\n'
    
    for i in range(length): # Print/identify other monotonic progress variables 
        if monoAry[i] == 2.0:
            if monoAryflag < 2:
                raise RuntimeError("Error in contents of monoAry vector.\n")
            elif monoAryflag == 2:
                print "Other candidate monotonic progress variables are:"
            otherC = iof.get_progvar(combosmatrix[1:,i], testspecies, locs, i)
            print 'C = %s' % otherC[1][0], 
            for j in otherC[1][1:]: 
                print "+ %s" % j,
            print "\n",
            monoAryflag = 3

    if monoAryflag < 1: # Give error if no best progress variable is found
        raise RuntimeError("Error: no best progress variable selected.")
    
    # Write results
    for i in range(nofiles):
        filesmatC.SetVal(i,0,progVar.GetVal(i,bestC[2]))

    # Plot results
    Cst = np.zeros(nofiles)
    Tst = np.zeros(nofiles)
    plt.figure()
    for ii in range(nofiles):
        Tst[ii] = progVar.GetVal(ii,0)
    if options["PlotAllC"][0] == 'yes':
        for jj in range(length-1):
            for ii in range(nofiles):
                Cst[ii] = progVar.GetVal(ii,jj+1)
            Cst = Cst/Cst.max()
            otherplt, = plt.plot(Tst, Cst, color='r')
    for ii in range(nofiles):
        Cst[ii] = filesmatC.GetVal(ii,0)
    Cst = Cst/Cst.max()
    bestplt, = plt.plot(Tst, Cst, color='k', marker='o', markerfacecolor='none')
    if options["PlotAllC"][0] == 'yes':    
        plt.legend([bestplt, otherplt],['Best progress variable','Other candidate progress variables'], loc='lower right')
    plt.xlabel("T (K)")
    plt.ylabel("Normalized Progress Variable (C/Cmax)")
    plt.title("Best Progress Variable")
    if skip == 'yes':
        plt.title("User-selected progress variable")
    plt.savefig("output/%s.pdf" % 'CvsTemp')
    plt.clf()

    return filesmatC
