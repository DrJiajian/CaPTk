#include <time.h>
#include "SBRT_Nodule.h"

#include "cbicaCmdParser.h"

int main( int argc, char** argv )
{
	clock_t processStart = clock();

	const int imageDimension = 3;	// image dimension, now support 3D only
	const int inputImageNum = 2;	// number of input images (modality), PET and CT in this application

	if (imageDimension != 3)
	{
		std::cerr << " Support 3D image only !" << std::endl;

		return EXIT_FAILURE;
	}

	cbica::CmdParser parser(argc, argv);
	parser.addRequiredParameter("p", "petImage", cbica::Parameter::STRING, "none", "Absolute path of PET image", "For example /data/.../pet.nii.gz");	
	parser.addRequiredParameter("c", "ctImage", cbica::Parameter::STRING, "none", "Absolute path of CT image", "For example /data/.../ct.nii.gz");
	parser.addRequiredParameter("m", "maskImage", cbica::Parameter::STRING, "none", "Absolute path of mask image", "For example /data/.../mask.nii.gz");
	parser.addRequiredParameter("o", "outputImage", cbica::Parameter::STRING, "none", "Absolute path and basename of output file (without extension)", "For example /output/.../label");
    parser.addOptionalParameter("l", "lungfieldLabel", cbica::Parameter::INTEGER, "none", "Label value of the lung field", "For example 2");
    parser.addOptionalParameter("s", "seedImage", cbica::Parameter::STRING, "none", "Absolute path of seed image", "For example /data/.../seed_img.nii.gz");
	parser.addOptionalParameter("L", "logFile", cbica::Parameter::STRING, "none", "Absolute path of log file", "For example log_file.txt");

  std::string petName, ctName, maskName, oName, seedName, logName;
  int seedAvail = 0;
  int lf_lab = 2;

	if (parser.isPresent("p"))
	{
		parser.getParameterValue("p", petName);
	}
	if (parser.isPresent("c"))
	{
		parser.getParameterValue("c", ctName);
	}
	if (parser.isPresent("m"))
	{
		parser.getParameterValue("m", maskName);
	}
	if (parser.isPresent("o"))
	{
		parser.getParameterValue("o", oName);
	}
    if (parser.isPresent("l"))
    {
        parser.getParameterValue("l", lf_lab);
    }
    if (parser.isPresent("s"))
    {
        parser.getParameterValue("s", seedName);
        seedAvail = 1;
    }
	if (parser.isPresent("L"))
	{
		parser.getParameterValue("L", logName);
	}

	std::vector<std::string> inputFileName;
	inputFileName.push_back(petName);
	inputFileName.push_back(ctName);

	//input image files, the order of input matters, 1st PET and 2nd CT
	//vector<string> inputFileName;
	//for ( int i=0;i<inputImageNum;i++ )
	//{
	//	inputFileName.push_back(argv[i+1]);
	//}

    //string maskName = argv[inputImageNum+1];	// lung field mask
    //string o_basename = argv[inputImageNum+2];	// output basename

	//float disThreshold = atof( argv[inputImageNum+3] );
	//float sigma = atof( argv[inputImageNum+4] );
	//float suvRatio = atof( argv[inputImageNum+5] );
	//int iterNum = atoi( argv[inputImageNum+6] );
	//int smoothIterNum = atoi( argv[inputImageNum+7] );
	//int smoothR = atoi( argv[inputImageNum+8] );
    //int minNumFgSeed = atoi( argv[inputImageNum+9] );

	std::vector<float> modalityWt(inputImageNum, 0.5);
	modalityWt[0] = 0.6;
	modalityWt[1] = 0.4;

	float disThreshold = 5.0;
	float sigma = 1.0;
	float suvRatio = 3.0;

	int iterNum = 200;
	int smoothIterNum = 3;
	int smoothR = 1;
	int minNumFgSeed = 25;

	//
	SBRT_Nodule< float,imageDimension,inputImageNum > segObject;

	if (!logName.empty())
	{
		segObject.SetLogger(logName);
	}

	segObject.SetParameters( lf_lab, oName, suvRatio, minNumFgSeed, modalityWt, disThreshold, sigma, iterNum, smoothIterNum, smoothR );
	segObject.Initialization( inputFileName, maskName );
    if (seedName.empty())
    {
        segObject.GenerateSeeds();
    }
    else
    {
        segObject.ReadSeedImage(seedName);
    }
	segObject.PerformSegmentation();
	segObject.WriteLabel(seedAvail);

	clock_t processEnd = clock();
	double processDuration = (double)(processEnd - processStart)/CLOCKS_PER_SEC;
	std::cout << "the whole process costs: " << processDuration << "s" << std::endl;

	return EXIT_SUCCESS;
}
