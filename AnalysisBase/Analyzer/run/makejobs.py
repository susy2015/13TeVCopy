#!/usr/bin/env python
import os
import sys
import re
import argparse
import subprocess
from ROOT import gROOT,TFile,TTree
gROOT.SetBatch(True)

# script to help with submission of ntupling jobs, either interactively or using a batch system (LSF on lxplus CERN or condor on cmslpc at FNAL)
# use ./makejobs.py -h to explore the options and usage
# edit datasets.conf to choose samples to submit
# run ./makejobs.py with the appropriate options
# then, submit jobs using ./submitall.sh
# to run the merging after the ntupling is done, run ./makejobs.py --runmerge --inputdir </path/to/dir/with/unmerged/files> -o </path/to/dir/with/merged/files>
# if you want to split the merged output into multiple files (for large samples), add the --splitmerge option, you'll be asked how many merged files you want and how many input files you want to merge per output file
# a script called runmerge.sh will be produced which will contain the commands needed to run the merging
# to run the postprocessing (adding cross section weights), run ./makejobs.py --postprocess -o </path/to/dir/with/merged/files> -c <conffile> -t <condor|lsf|interactive>

parser = argparse.ArgumentParser(description='Prepare and submit ntupling jobs')
parser.add_argument("--postprocess", dest="postprocess", action='store_true', help="Run postprocessing instead of job submission. [Default: False]")
parser.add_argument("--treename", dest="treename", default="TestAnalyzer/Events", help="Name of trees in merged input files, needed for postprocessing. [Default: TestAnalyzer/Events]")
parser.add_argument("--lumi", dest="lumi", type=float, default=1., help="Integrated luminosity to be used for calculating cross section weights in postprocessing. [Default: 1.]")
parser.add_argument("--postsuffix", dest="postsuffix", default="wgtxsec", help="Suffix to add to output files from postprocessing. [Default: wgtxsec]")
parser.add_argument("--runmerge", dest="runmerge", action='store_true', help="Run file merging instead of job submission. [Default: False]")
parser.add_argument("--splitmerge", dest="splitmerge", action='store_true', help="Split file merging into multiple output files. [Default: False]")
parser.add_argument("--inputdir", dest="inputdir", default="/store/user/${USER}/13TeV/ntuples", help="Input directory with unmerged ntuples. [Default: \"/store/user/${USER}/13TeV/ntuples\"]")
parser.add_argument("-i", "--input", dest="input", default="datasets.conf", help="Input configuration file with list of datasets. [Default: datasets.conf]")
parser.add_argument("-s", "--submit", dest="submit", default="submitall", help="Name of shell script to run for job submission. [Default: submitall]")
parser.add_argument("-n", "--numperjob", dest="numperjob", type=int, default=5, help="Number of files or events per job. Splittype determines whether splitting is by number of files (default) or by number of events. [Default: 5]")
parser.add_argument("-m", "--maxevents", dest="maxevents", type=int, default=-1, help="Maximum number of events to run over. [Default: -1 (all)]")
parser.add_argument("-p", "--pathtocfg", dest="path", default="../test", help="Path to directory with python configuration file to be run using cmsRun. [Default: \"../test/\"]")
parser.add_argument("-c", "--config", dest="config", default="runTestAnalyzer.py", help="Configuration file to be run using cmsRun to run. [Default: runTestAnalyzer.py]")
parser.add_argument("-d", "--dbsinstance", dest="dbsinstance", default="prod/global", help="DBS instance to query for dataset. [Default: prod/global]")
parser.add_argument("-o", "--outdir", dest="outdir", default="/store/user/${USER}/13TeV/ntuples", help="Output directory for ntuples. [Default: \"/store/user/${USER}/13TeV/ntuples\"]")
parser.add_argument("-j", "--jobdir", dest="jobdir", default="jobs", help="Directory for job files. [Default: jobs]")
parser.add_argument("-r", "--runscript", dest="script", default="runjobs", help="Shell script to be run by the jobs, [Default: runjobs]")
parser.add_argument("-t", "--submittype", dest="submittype", default="condor", choices=["interactive","lsf","condor"], help="Method of job submission. [Options: interactive, lsf, condor. Default: condor]")
parser.add_argument("-l", "--splittype", dest="splittype", default="file", choices=["file","event"], help="Split jobs by number of files or events. [Options: file, event. Default: file]")
parser.add_argument("-q", "--queue", dest="queue", default="8nh", help="LSF submission queue. [Default: 8nh]")
parser.add_argument("-a", "--arrangement", dest="arrangement", default="das", choices=["das","local"], help="(ntuplizing only) Specifies if samples' paths are das, or local eos space (format: /eos/uscms/store/... or /eos/cms/store/...). If local, then file-based splitting required, and sample name will be used to discover files (eg \'find /eos/uscms/store/...\') [Options: das, local. Default: das]")
#parser.print_help()
args = parser.parse_args()

def get_num_events(filelist,prefix='',wgtsign=1,treename='TestAnalyzer/Events'):
    totentries = 0
    for filename in filelist :
        filepath = '/'.join(['%s' % prefix,'%s' % filename])
        file = TFile.Open(filepath)
        tree = file.Get(treename)
        if wgtsign > 0 :
            totentries += tree.GetEntries('genweight>0')
        else :
            totentries += tree.GetEntries('genweight<0')
    return totentries

processes = []
samples = []
xsecs = []
datasets = []
totnposevents = []
totnnegevents = []

with open(args.input,"r") as f :
    for line in f :
        content = line.split()
        if "#" in content[0] :
            continue
        processes.append(content[0])
        samples.append(content[1])
        xsecs.append(content[2])
        datasets.append(content[3])

eos="/afs/cern.ch/project/eos/installation/0.3.15/bin/eos.select"

os.system("mkdir -p %s" % args.jobdir)

if args.postprocess : 
    files = []
    prefix = ""
    for isam in range(len(samples)) :
        filelist = []
        if args.outdir.startswith("/eos/cms/store/user") or args.outdir.startswith("/store/user") :
            cmd = ("%s find -f %s | egrep '%s(_[0-9]+|)_ntuple.root'" % (eos,args.outdir,samples[isam]))
            ps = subprocess.Popen(cmd,shell=True,stdout=subprocess.PIPE,stderr=subprocess.PIPE)
            result = ps.communicate()
            filelist = result[0].rstrip('\n').split('\n')
            prefix = "root://eoscms/"
        else :
            filelist = [os.path.join(args.outdir, f) for f in os.listdir(args.outdir) if re.match(r'%s(_[0-9]+|)_ntuple.root' % samples[isam], f)]
            if args.outdir.startswith("/eos/uscms/store/user") :
                prefix = "root://cmseos:1094/"
        files.append(filelist)
        nposevents = get_num_events(filelist,prefix)
        nnegevents = get_num_events(filelist,prefix,-1)
        totnposevents.append(nposevents)
        totnnegevents.append(nnegevents)
        print "Sample "+samples[isam]+" has "+str(nposevents)+" positive and "+str(nnegevents)+" negative weight events"

    print "Creating submission file: submit_addweight.sh"
    script = open("submit_addweight.sh","w")
    script.write("""#!/bin/bash
outputdir={outdir}
runmacro=AddWgt2UCSBntuples.C
treename={tname}
prefix={prefixs}
suffix={suffixs}
""".format(outdir=args.outdir, tname=args.treename, prefixs=prefix, suffixs=args.postsuffix))

    if args.submittype == "lsf" or args.submittype == "condor" :
        script.write("""
workdir=$CMSSW_BASE
runscript=runaddweight{stype}.sh

if [ ! "$CMSSW_BASE" ]; then
  echo "-------> error: define cms environment."
  exit 1
fi

cp AddWgt2UCSBntuples.C $workdir
cp rootlogon.C $workdir

echo "$runscript $runmacro $workdir $outputdir"
""".format(stype=args.submittype))
    
    for isam in range(len(samples)) :
        for ifile in range(len(files[isam])) :
            filename = files[isam][ifile].split('/')[-1]
            outfilename = filename.replace(".root","_{}.root".format(args.postsuffix))
            if args.submittype == "interactive" :
                script.write("""root -l -q -b $runmacro+\(\\"$prefix{fname}\\",\\"{process}\\",{xsec},{lumi},{nposevts},{nnegevts},\\"$treename\\",\\"$suffix\\"\)\n""".format(
                fname=files[isam][ifile], process=processes[isam], xsec=xsecs[isam], lumi=args.lumi, nposevts=totnposevents[isam], nnegevts=totnnegevents[isam]
                ))
            elif args.submittype == "lsf" :
                script.write("""bsub -q {queue} $runscript $runmacro $prefix{fname} {process} {xsec} {lumi} {nposevts} {nnegevts} $treename $suffix {outname} {outdir} $workdir\n""".format(
                queue=args.queue, fname=files[isam][ifile], process=processes[isam], xsec=xsecs[isam], lumi=args.lumi, nevts=totnevents[isam], outname=outfilename, outdir=args.outdir
                ))
            elif args.submittype == "condor" :
                os.system("mkdir -p %s/logs" % args.jobdir)
                jobscript = open("{}/submit_{}_{}_addwgt.sh".format(args.jobdir,samples[isam],ifile),"w")
                jobscript.write("""
cat > submit.cmd <<EOF
universe                = vanilla
Requirements            = (Arch == "X86_64") && (OpSys == "LINUX")
request_disk            = 10000000
request_memory          = 199
Executable              = runaddweight{stype}.sh
Arguments               = {macro} {prefixs}{fname} {process} {xsec} {lumi} {nposevts} {nnegevts} {tname} {suffix} {outname} {outdir} {workdir}
Output                  = logs/{sname}_{num}_addweight.out
Error                   = logs/{sname}_{num}_addweight.err
Log                     = logs/{sname}_{num}_addweight.log
use_x509userproxy       = true
initialdir              = {jobdir}
Should_Transfer_Files   = YES
transfer_input_files    = {workdir}/{macro},{workdir}/rootlogon.C
WhenToTransferOutput    = ON_EXIT
Queue
EOF

condor_submit submit.cmd;
rm submit.cmd""".format(
                stype=args.submittype, macro="AddWgt2UCSBntuples.C", prefixs=prefix, workdir="${CMSSW_BASE}", fname=files[isam][ifile], process=processes[isam], xsec=xsecs[isam], lumi=args.lumi, nposevts=totnposevents[isam], nnegevts=totnnegevents[isam], tname=args.treename, suffix=args.postsuffix, sname=samples[isam], num=ifile, jobdir=args.jobdir, outname=outfilename, outdir=args.outdir
                ))
                jobscript.close()
                script.write("./{jobdir}/submit_{name}_{j}_addwgt.sh\n".format(jobdir=args.jobdir,name=samples[isam], j=ifile))
                os.system("chmod +x %s/submit_%s_%d_addwgt.sh" %(args.jobdir,samples[isam], ifile))


    script.close()
    os.system("chmod +x submit_addweight.sh")

    print "Done!"
    exit()

elif args.runmerge :

    if args.outdir.startswith("/eos/cms/store/user") or args.outdir.startswith("/store/user") :
        os.system("%s mkdir -p %s" % (eos,args.outdir))
    else :
        os.system("mkdir -p %s" % args.outdir)

    prefix = ""
    mergecommands = []
    for isam in range(len(samples)) :
        filelist = []
        if args.inputdir.startswith("/eos/cms/store/user") or args.inputdir.startswith("/store/user") :
            prefix = "root://eoscms/"
            filelist.append
            cmd = ("%s find -f %s | egrep 'output_[0-9]+_%s(_numEvent[0-9]+|).root'" % (eos,args.inputdir,samples[isam]))
            ps = subprocess.Popen(cmd,shell=True,stdout=subprocess.PIPE,stderr=subprocess.PIPE)
            result = ps.communicate()
            filelist = result[0].rstrip('\n').split('\n')
            filelist = ["".join([prefix,file]) for file in filelist]
            outname = "%s%s/%s_ntuple.root" % (prefix,args.outdir,samples[isam])
            filelist.insert(0, outname)
        else :
            filelist = [os.path.join(args.inputdir, f) for f in os.listdir(args.inputdir) if re.match(r'output_[0-9]+_%s(_numEvent[0-9]+|).root' % samples[isam], f)]
            if args.inputdir.startswith("/eos/uscms/store/user") :
                prefix = "root://cmseos:1094/"
            filelist = ["".join([prefix,file]) for file in filelist]
            outname = "%s%s/%s_ntuple.root" % (prefix,args.outdir,samples[isam])
            filelist.insert(0, outname)
        mergefile = "merge_%s.txt" % samples[isam]
        with open(mergefile,"w") as f:
            f.write('\n'.join(filelist))
        print "There are %d files to merge for %s\n" % (len(filelist)-1, samples[isam])
        if args.splitmerge :
            nummergedfiles = int(input("How many merged files do you want? "))
            numfilespermerge = int(input("How many input files per merge? "))
            for imerge in range(1,nummergedfiles+1) :
                start = ((imerge-1)*numfilespermerge) + 2
                end   = (imerge*numfilespermerge) + 1
                outfile = "merge_%s_%d.txt" % (samples[isam],imerge)
                os.system("sed -n 1p %s | sed \"s/ntuple.root/%d_ntuple.root/\" > %s" % (mergefile,imerge,outfile))
                os.system("sed -n %d,%dp %s >> %s" % (start,end,mergefile,outfile))
                mergecommands.append("root -l -q -b MergeNtuples.C+\\(\\\"%s\\\"\\)" % outfile)
            rem = len(filelist) - 1 - (nummergedfiles*numfilespermerge)
            if rem > 0 :
                print "Will add last %d files to last merged file" % rem
                os.system("tail -n %d %s >> merge_%s_%d.txt" % (rem,mergefile,samples[isam],nummergedfiles))
        else :
            mergecommands.append("root -l -q -b MergeNtuples.C+\\(\\\"%s\\\"\\)" % mergefile)

    with open("submitmerge.sh","w") as f :
        f.write("#!/bin/bash\n")
        f.write('\n'.join(mergecommands))
        f.write("\n")
    os.system("chmod +x submitmerge.sh")

else :

    if args.outdir.startswith("/eos/cms/store/user") or args.outdir.startswith("/store/user") :
        os.system("%s mkdir -p %s" % (eos,args.outdir))
    else :
        os.system("mkdir -p %s" % args.outdir)

    maxevts = args.maxevents if args.splittype == "file" else args.numperjob

    print "Creating submission file: ",args.submit+".sh"
    script = open(args.submit+".sh","w")
    script.write("""#!/bin/bash
outputdir={outdir}
jobdir={jobdir}
cfgfile={cfg}
numperjob={numperjob}
maxevents={maxevents}
""".format(outdir=args.outdir, jobdir=args.jobdir, pathtocfg=args.path, cfg=args.config, numperjob=args.numperjob, maxevents=maxevts))

    if args.submittype == "lsf" or args.submittype == "condor" :
        script.write("""
workdir=$CMSSW_BASE
runscript={runscript}{stype}.sh

if [ ! "$CMSSW_BASE" ]; then
  echo "-------> error: define cms environment."
  exit 1
fi

cp {pathtocfg}/$cfgfile $workdir

echo "$cfgfile $runscript $workdir $outputdir"
""".format(
        pathtocfg=args.path,runscript=args.script,stype=args.submittype
        ))
    for isam in range(len(samples)) :

 	if args.arrangement == "das" :
		cmd = ("./das_client.py --query \"file dataset=%s instance=%s\" --limit=0 | grep store > %s/filelist_%s.txt" % (datasets[isam],args.dbsinstance,args.jobdir,samples[isam]))
	elif args.arrangement == "local" and args.splittype == "file" :
		cmd = ("find %s | grep .root | sort -V > %s/filelist_%s.txt" % (datasets[isam],args.jobdir,samples[isam]))
	else :
		print "Fatal error: File-based splitting (splittype == \"file\") required if local."
        subprocess.call(cmd,shell=True)
        if args.arrangement == "das" :
		cmd = ("./das_client.py --query \"dataset=%s instance=%s| grep dataset.nevents\" | sed -n 4p | tr -d '\n'" % (datasets[isam],args.dbsinstance))
        	ps = subprocess.Popen(cmd,shell=True,stdout=subprocess.PIPE)
        	numevents=int(ps.communicate()[0])
        	if args.maxevents > -1  and args.maxevents < numevents :
            		numevents = args.maxevents
        numperjob = args.numperjob
        numlines = 0
	infilename = "%s/filelist_%s.txt" % (args.jobdir,samples[isam])
        print "Creating jobs for %s" % samples[isam]
        with open(infilename,"r") as infile :
           numlines = len(infile.readlines())
        if args.splittype == "file" :
            numjobs = int(numlines)/int(numperjob)
            rem = numlines % numperjob
        elif args.splittype == "event" :
            numjobs = int(numevents)/int(numperjob)
            rem = numevents % numperjob
        if rem == 0 :
            numjobs -= 1
        if args.splittype == "file" :
            print "\t %d files to process --> will produce %d jobs with %d files per job" % (numlines,(numjobs+1),numperjob)
        elif args.splittype == "event" :
            print "\t %d events to process --> will produce %d jobs with %d events per job" % (numevents,(numjobs+1),numperjob)
        for ijob in range(numjobs+1) :
            start = (ijob*numperjob) + 1
            end = numperjob*(ijob+1)
            jobfile = "filelist_%s.txt" % (samples[isam])
            skipevts = 0
            if args.splittype == "file" and args.arrangement == "local" :
                jobfile = "job_%d_%s.txt" % (ijob,samples[isam])
                os.system("sed -n %d,%dp %s | awk \'{print \"file:\" $0}\' > %s/%s" % (start,end,infilename,args.jobdir,jobfile))
 	    elif args.splittype == "file" :
                jobfile = "job_%d_%s.txt" % (ijob,samples[isam])
                os.system("sed -n %d,%dp %s > %s/%s" % (start,end,infilename,args.jobdir,jobfile))
            elif args.splittype == "event" :
                skipevts = start-1
            outfile = "output_%d_%s.root" % (ijob,samples[isam])
            suffix = "_numEvent{}".format(maxevts) if maxevts > -1 else ""
            cmd = ""
            if args.outdir.startswith("/eos/cms/store/user") or args.outdir.startswith("/store/user") :
                cmd = ("%s ls %s | grep -c output_%d_%s%s.root" % (eos,args.outdir,ijob,samples[isam],suffix))
	    else :
                cmd = "ls %s | grep -c output_%d_%s%s.root" % (args.outdir,ijob,samples[isam],suffix)
            ps = subprocess.Popen(cmd,shell=True,stdout=subprocess.PIPE)
            output = ps.communicate()[0][0]
            if int(output) > 0 :
                print "Output file output_%s_%d%s.root exists already! Skipping ..." % (samples[isam],ijob,suffix)
                continue

            if args.submittype == "interactive" :
                os.system("mkdir -p %s" % args.outdir)
                script.write("""cmsRun {pathtocfg}/$cfgfile print inputFiles_clear inputFiles_load=$jobdir/{infile} outputFile=$outputdir/{outputname} maxEvents={maxevents} skipEvents={skipevents}\n""".format(
                pathtocfg=args.path,infile=jobfile,outputname=outfile,maxevents=maxevts,skipevents=skipevts
                ))

            elif args.submittype == "lsf" :
                cpinput = ""
                if args.splittype == "file" or (args.splittype == "event" and ijob == 0) :
                    cpinput = "\ncp $jobdir/%s $workdir \n" % (jobfile)
                script.write("""{cptxt}
    bsub -q {queue} $runscript $cfgfile {infile} $outputdir {outputname} {maxevents} {skipevents} $workdir \n""".format(
                cptxt=cpinput,queue=args.queue,infile=jobfile,outputname=outfile,maxevents=maxevts,skipevents=skipevts
                ))

            elif args.submittype == "condor" :
                os.system("mkdir -p %s/logs" % args.jobdir)
                jobscript = open("{0}/submit_{1}_{2}.sh".format(args.jobdir,samples[isam],ijob),"w")
                jobscript.write("""
cat > submit.cmd <<EOF
universe                = vanilla
Requirements            = (Arch == "X86_64") && (OpSys == "LINUX")
request_disk            = 10000000
request_memory          = 199
Executable              = {runscript}{stype}.sh
Arguments               = {cfg} {infile} {outputdir} {outputname} {maxevents} {skipevents} {workdir}
Output                  = logs/{sname}_{num}.out
Error                   = logs/{sname}_{num}.err
Log                     = logs/{sname}_{num}.log
use_x509userproxy       = true
initialdir              = {jobdir}
Should_Transfer_Files   = YES
transfer_input_files    = {workdir}/{cfg},{workdir}/{infile}
WhenToTransferOutput    = ON_EXIT
Queue
EOF

condor_submit submit.cmd;
rm submit.cmd""".format(
                runscript=args.script, stype=args.submittype, pathtocfg=args.path, cfg=args.config, infile=jobfile, workdir="${CMSSW_BASE}", sname=samples[isam], num=ijob, jobdir=args.jobdir, outputdir=args.outdir, outputname=outfile, maxevents=maxevts, skipevents=skipevts, evttag=suffix
                ))
                jobscript.close()
                cpinput = ""
                if args.splittype == "file" or (args.splittype == "event" and ijob == 0) :
                    cpinput = "\ncp $jobdir/%s $workdir \n" % (jobfile)
                script.write("{cptxt}./$jobdir/submit_{name}_{j}.sh\n".format(cptxt=cpinput, name=samples[isam], j=ijob))
                os.system("chmod +x %s/submit_%s_%d.sh" %(args.jobdir, samples[isam], ijob))


    script.close()
    os.system("chmod +x %s.sh" % args.submit)

    print "Done!"
