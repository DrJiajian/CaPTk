#include "DiffusionDerivatives.h"
#include "cbicaUtilities.h"
#include "cbicaCmdParser.h"

int main(int argc, char **argv)
{
  cbica::CmdParser parser = cbica::CmdParser(argc, argv, "DiffusionDerivatives");
  parser.addRequiredParameter("i", "input", cbica::Parameter::STRING, "", "The input DWI file.");
  parser.addRequiredParameter("m", "mask", cbica::Parameter::STRING, "", "The input mask file.");
  parser.addRequiredParameter("b", "Bval", cbica::Parameter::STRING, "", "The input bval file.");
  parser.addRequiredParameter("v", "Bvec", cbica::Parameter::STRING, "", "The input bvec file.");

  parser.addRequiredParameter("a", "Axial Diffusivity", cbica::Parameter::STRING, "", "The Axial Diffusivity image (1=YES, 0=NO, 1 (Default))");
  parser.addRequiredParameter("f", "Fractional Anisotropy", cbica::Parameter::STRING, "", "The Fractional Anisotropy image (1=YES, 0=NO, 1 (Default))");
  parser.addRequiredParameter("r", "Radial Diffusivity", cbica::Parameter::STRING, "", "The Radial Diffusivity image (1=YES, 0=NO, 1 (Default))");
  parser.addRequiredParameter("t", "Apparent Diffusion Coefficient", cbica::Parameter::STRING, "", "The Apparent Diffusion Coefficient (1=YES, 0=NO, 1 (Default))");

  parser.addRequiredParameter("o", "output", cbica::Parameter::STRING, "", "The output directory.");
  parser.addOptionalParameter("L", "Logger", cbica::Parameter::STRING, "log file which user has write access to", "Full path to log file to store console outputs", "By default, only console output is generated");

  // parameters to get from the command line
  cbica::Logging logger;
  std::string loggerFile;
  bool loggerRequested = false;
  bool axPresent = 1;
  bool faPresent = 1;
  bool radPresent = 1;
  bool trPresent = 1;

  int tempPosition;
  std::string inputFileName, inputMaskName, inputBValName, inputBVecName, outputDirectoryName;

  if ((argc < 1) || (parser.compareParameter("u", tempPosition)))
  {
    parser.echoUsage();
    return EXIT_SUCCESS;
  }
  if (parser.compareParameter("L", tempPosition))
  {
    loggerFile = argv[tempPosition + 1];
    loggerRequested = true;
    logger.UseNewFile(loggerFile);
  }
  if (parser.compareParameter("i", tempPosition))
  {
    inputFileName = argv[tempPosition + 1];
  }

  if (parser.compareParameter("m", tempPosition))
  {
    inputMaskName = argv[tempPosition + 1];
  }
  if (parser.compareParameter("b", tempPosition))
  {
    inputBValName = argv[tempPosition + 1];
  }
  if (parser.compareParameter("v", tempPosition))
  {
    inputBVecName = argv[tempPosition + 1];
  }


  if (parser.compareParameter("a", tempPosition))
  {
    parser.getParameterValue("a", axPresent);
  }
  if (parser.compareParameter("f", tempPosition))
  {
    parser.getParameterValue("f", faPresent);
  }
  if (parser.compareParameter("r", tempPosition))
  {
    parser.getParameterValue("r", radPresent);
  }
  if (parser.compareParameter("t", tempPosition))
  {
    parser.getParameterValue("t", trPresent);
  }

  if (parser.compareParameter("o", tempPosition))
  {
    outputDirectoryName = argv[tempPosition + 1];
  }

  std::cout << "Input File:" << inputFileName << std::endl;
  std::cout << "Input Mask:" << inputMaskName << std::endl;
  std::cout << "Input BVal:" << inputBValName << std::endl;
  std::cout << "Input BVec:" << inputBVecName << std::endl;
  std::cout << "Output Directory:" << outputDirectoryName << std::endl;

  if (!cbica::isFile(inputFileName))
  {
    std::cout << "The input file does not exist:" << inputFileName << std::endl;
    return EXIT_FAILURE;
  }
  if (!cbica::isFile(inputMaskName))
  {
    std::cout << "The input mask does not exist:" << inputMaskName << std::endl;
    return EXIT_FAILURE;
  }
  if (!cbica::isFile(inputBValName))
  {
    std::cout << "The input bval file does not exist:" << inputBValName << std::endl;
    return EXIT_FAILURE;
  }
  if (!cbica::isFile(inputBVecName))
  {
    std::cout << "The input bvec file does not exist:" << inputBVecName << std::endl;
    return EXIT_FAILURE;
  }
  if (!cbica::directoryExists(outputDirectoryName))
  {
    if (!cbica::createDirectory(outputDirectoryName))
      std::cout << "The output directory can not be created:" << outputDirectoryName << std::endl;
    return EXIT_FAILURE;
  }
  DiffusionDerivatives objDiffusion;
  std::vector<typename ImageTypeFloat3D::Pointer> diffusionDerivatives = objDiffusion.Run(inputFileName, inputMaskName, inputBVecName, inputBValName, outputDirectoryName);
  std::cout << "Writing measures to the specified output directory.\n";

  //fa,tr, rad , ax
  if (faPresent== true)
    cbica::WriteImage< ImageTypeFloat3D >(diffusionDerivatives[0], outputDirectoryName + "/FractionalAnisotropy.nii.gz");
  if (trPresent == true)
    cbica::WriteImage< ImageTypeFloat3D >(diffusionDerivatives[1], outputDirectoryName + "/ApparentDiffusionCoefficient.nii.gz");
  if (radPresent == true)
    cbica::WriteImage< ImageTypeFloat3D >(diffusionDerivatives[2], outputDirectoryName + "/RadialDiffusivity.nii.gz");
  if (axPresent == true)
    cbica::WriteImage< ImageTypeFloat3D >(diffusionDerivatives[3], outputDirectoryName + "/AxialDiffusivity.nii.gz");

  std::cout << "Finished successfully.\n";
  std::cout << "\nPress any key to continue............\n";

  int input;
  std::cin >> input;
  return EXIT_SUCCESS;
}