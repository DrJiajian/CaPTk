/**
\file  featureExtraction.cxx

\brief Feature Extraction comand line interface.

Dependecies: ITK (module_review, module_skullstrip enabled)

https://www.cbica.upenn.edu/sbia/software/ <br>
software@cbica.upenn.edu

Copyright (c) 2015 University of Pennsylvania. All rights reserved. <br>
See COPYING file or https://www.cbica.upenn.edu/sbia/software/license.html

*/

#include "cbicaCmdParser.h"
#include "cbicaLogging.h"
#include "cbicaITKSafeImageIO.h"
#include "cbicaUtilities.h"
#include "itkImageFileReader.h"
#include "itkImageFileWriter.h"
#include "FeatureExtraction.h"
#include "itkDOMNodeXMLReader.h"
#include "itkDOMNodeXMLWriter.h"
#include "itkDOMNode.h"

//#include "cbicaITKImageInfo.h"

// stuff used in the program
std::string loggerFile, multipatient_file, patient_id, image_path_string, modalities_string, maskfilename, 
selected_roi_string, roi_labels_string, param_file, outputdir, offset_String;

bool debug = false, debugWrite = false, verticalConc = false, featureMaps = false;

int threads = 1;

std::vector< std::string > modality_names, image_paths, selected_roi, roi_labels;


//! The main algorithm, which is templated across the image type
template< class TImageType >
void algorithmRunner()
{
  FeatureExtraction<TImageType> features;
  std::vector<typename TImageType::Pointer> inputimages;

  if (!cbica::isFile(maskfilename))
  {
    std::cerr << "Mask file is needed [use parameter '-m' on command line for single subject or the header 'ROIFile' in batch file].\n";
    exit(EXIT_FAILURE);
  }
  if (image_paths.empty())
  {
    std::cerr << "Input images are needed [use parameter '-i' on command line for single subject or the header 'Images' in batch file].\n";
    exit(EXIT_FAILURE);
  }
  if (modality_names.empty())
  {
    std::cerr << "Modality name(s) is needed [use parameter '-t' on command line for single subject or the header 'Modalities' in batch file].\n";
    exit(EXIT_FAILURE);
  }
  if (image_paths.size() != modality_names.size())
  {
    std::cerr << "Number of images and modalities should be the same.\n";
    exit(EXIT_FAILURE);
  }
  if (patient_id.empty())
  {
    std::cerr << "Patient name or ID is needed [use parameter '-n' on command line for single subject or the header 'PATIENT_ID' in batch file].\n";
    exit(EXIT_FAILURE);
  }
  if (selected_roi.empty())
  {
    std::cout << "No ROI values have been selected for patient_id '" << patient_id << "', computation shall be done on all ROIs present in mask.\n";
    //selected_roi = "all";
  }
  if (roi_labels.empty())
  {
    std::cout << "No ROI labels have been provided for patient_id '" << patient_id << "', the ROI values will be used as labels instead.\n";
    //roi_labels = "all";
  }

  std::vector< std::string > imageNames = image_paths;

  //check if all the input images and mask match dimension spacing and size
  for (size_t i = 0; i < imageNames.size(); i++)
  {
    if (cbica::isDir(imageNames[i]))
    {
      std::cerr << "Images cannot have directory input. Please use absolute paths.\n";
      exit(EXIT_FAILURE);
    }
    if (!cbica::ImageSanityCheck(imageNames[i], maskfilename))
    {
      exit(EXIT_FAILURE);
    }
    inputimages.push_back(cbica::ReadImage< TImageType >(imageNames[i]));
  }

  if (inputimages.size() != imageNames.size())
  {
    std::cerr << "Not all images are in the same space as the mask, only those that pass this sanity check will be processed.\n";
  }

  if (debug)
  {
    std::cout << "[DEBUG] Initializing FE class.\n";
    features.EnableDebugMode();
  }
  if (debugWrite)
  {
    features.EnableWritingOfIntermediateFiles();
  }

  features.SetPatientID(patient_id);
  features.SetInputImages(inputimages, modality_names);
  features.SetSelectedROIsAndLabels(selected_roi, roi_labels);

  // check if the provided labels are present mask image. if not exit the program 
  typename TImageType::Pointer mask = cbica::ReadImage< TImageType >(maskfilename);
  typedef itk::ImageRegionIterator< TImageType > IteratorType;
  IteratorType  imageIterator(mask, mask->GetBufferedRegion());

  if (debug)
  {
    std::cout << "[DEBUG] Checking if selected ROIs are present in mask or not.\n";
  }
  auto selectedROIs = features.GetSelectedROIs();
  if (selectedROIs.size() != 0)
  {
    for (size_t x = 0; x < selectedROIs.size(); x++)
    {
      imageIterator.GoToBegin();
      while (!imageIterator.IsAtEnd())
      {
        if (imageIterator.Get() == selectedROIs[x])
          break;
        ++imageIterator;
        if (imageIterator.IsAtEnd())
        {
          std::cout << "The ROI for calculation, '" << std::to_string(selectedROIs[x]) << "' does not exist in the mask, '" << maskfilename << "'.\n";
          exit(EXIT_FAILURE);
        }
      }
    }
  }
  features.SetValidMask();
  features.SetMaskImage(mask);
  features.SetRequestedFeatures(param_file);
  features.SetOutputFilename(cbica::normPath(outputdir));
  features.SetVerticallyConcatenatedOutput(verticalConc);
  features.SetWriteFeatureMaps(featureMaps);
  features.SetNumberOfThreads(threads);
  features.Update();
}

//! Calls cbica::stringSplit() by checking for both "," and "|" as deliminators
std::vector< std::string > splitTheString(const std::string &inputString)
{
  std::vector< std::string > returnVector;
  if (inputString.find(",") != std::string::npos)
  {
    returnVector = cbica::stringSplit(inputString, ",");
  }
  else if (inputString.find("|") != std::string::npos)
  {
    returnVector = cbica::stringSplit(inputString, "|");
  }
  else if(!inputString.empty()) // only a single value is present
  {
    returnVector.push_back(inputString);
  }
  return returnVector;
}

int main(int argc, char** argv)
{
  cbica::CmdParser parser(argc, argv);

  parser.addOptionalParameter("p", "paramFile", cbica::Parameter::FILE, ".csv", "A csv file with all features and its parameters filled", "Default: '../data/1_params_default.csv'");
  parser.addRequiredParameter("o", "outputDir", cbica::Parameter::DIRECTORY, "none", "Absolute path of directory to save results", "Result can be a CSV or Feature Maps (for lattice)");

  parser.addOptionalParameter("b", "batchFile", cbica::Parameter::FILE, ".csv", "Input file with Multi-Patient Multi-Modality details", "Header format is as follows:", "'PATIENT_ID,IMAGES,MASK,ROI,SELECTED_ROI,ROI_LABEL,", "SELECTED_FEATURES,PARAM_FILE'", "Delineate individual fields by '|'");

  parser.addOptionalParameter("n", "name_patient", cbica::Parameter::STRING, "none", "Patient id", "Required for single subject mode");
  parser.addOptionalParameter("i", "imagePaths", cbica::Parameter::STRING, "none", "Absolute path of each coregistered modality", "Delineate by ','", "Example: -i c:/test1.nii.gz,c:/test2.nii.gz", "Required for single subject mode");
  parser.addOptionalParameter("t", "imageTypes", cbica::Parameter::STRING, "none", "Names of modalities to be processed", "Delineate by ','", "Example: -t T1,T1Gd", "Required for single subject mode");
  parser.addOptionalParameter("m", "maskPath", cbica::Parameter::STRING, "none", "Absolute path of mask coregistered with images", "Required for single subject mode");
  parser.addOptionalParameter("r", "roi", cbica::Parameter::STRING, "none", "List of roi for which feature extraction is to be done", "Delineate by ','", "Example: -r 1,2", "Required for single subject mode");
  parser.addOptionalParameter("l", "labels", cbica::Parameter::STRING, "none", "Labels variables for selected roi numbers", "Delineate by ','", "Usage: -l Edema,Necrosis", "Required for single subject mode");
  parser.addOptionalParameter("vc", "verticalConc", cbica::Parameter::BOOLEAN, "flag", "Whether vertical concatenation is needed or not", "Horizontal concatenation is useful for training", "Defaults to '0'");
  parser.addOptionalParameter("f", "featureMapsWrite", cbica::Parameter::BOOLEAN, "flag", "Whether downsampled feature maps are written or not", "For Lattice computation ONLY", "Defaults to '0'");
  parser.addOptionalParameter("th", "threads", cbica::Parameter::INTEGER, "1-64", "Number of (OpenMP) threads to run FE on", "Defaults to '1'", "This gets disabled when lattice is disabled");
  parser.addOptionalParameter("of", "offsets", cbica::Parameter::STRING, "none", "Exact offset values to pass on for GLCM & GLRLM", "Should be same as ImageDimension and in the format '<offset1>,<offset2>,<offset3>'", "This is scaled on the basis of the radius", "Example: '-of 0x0x1,0x1x0'");

  parser.addOptionalParameter("d", "debug", cbica::Parameter::BOOLEAN, "true or false", "Whether to print out additional debugging info", "Defaults to '0'");
  parser.addOptionalParameter("dw", "debugWrite", cbica::Parameter::BOOLEAN, "true or false", "Whether to write intermediate files or not", "Defaults to '0'");
  parser.addOptionalParameter("L", "Logger", cbica::Parameter::FILE, "Text file with write access", "Full path to log file to store logging information", "By default, only console output is generated");

  //bool loggerRequested = false;
  //cbica::Logging logger;

  if (parser.isPresent("i"))
  {
    parser.getParameterValue("i", image_path_string);
    if (cbica::isDir(image_path_string))
    {
      std::cerr << "Images cannot handle a directory input, please use absolute paths.\n";
      exit(EXIT_FAILURE);
    }
    image_paths = splitTheString(image_path_string);
  }

  if (parser.isPresent("r"))
  {
    parser.getParameterValue("r", selected_roi_string);
    selected_roi = splitTheString(selected_roi_string);
  }

  if (parser.isPresent("vc"))
  {
    parser.getParameterValue("vc", verticalConc);
  }

  if (parser.isPresent("d"))
  {
    parser.getParameterValue("d", debug);
  }

  if (parser.isPresent("dw"))
  {
    parser.getParameterValue("dw", debugWrite);
  }

  if (parser.isPresent("f"))
  {
    parser.getParameterValue("f", featureMaps);
  }

  if (parser.isPresent("th"))
  {
    parser.getParameterValue("th", threads);
  }

  if (parser.isPresent("l"))
  {
    parser.getParameterValue("l", roi_labels_string);
    roi_labels = splitTheString(roi_labels_string);
  }

  if (parser.isPresent("t"))
  {
    parser.getParameterValue("t", modalities_string);
    modality_names = splitTheString(modalities_string);
  }

  if (parser.isPresent("m"))
  {
    parser.getParameterValue("m", maskfilename);
    if (cbica::isDir(maskfilename))
    {
      std::cerr << "Mask File cannot handle a directory input, please use absolute paths.\n";
      exit(EXIT_FAILURE);
    }
  }

  //if (parser.isPresent("L"))
  //{
  //  parser.getParameterValue("L", loggerFile);
  //  loggerRequested = true;
  //  logger.UseNewFile(loggerFile);
  //}

  if (parser.isPresent("o"))
  {
    parser.getParameterValue("o", outputdir);
  }

  if (parser.isPresent("p"))
  {
    parser.getParameterValue("p", param_file);
    if (!cbica::fileExists(param_file))
    {
      std::cerr << "The param file provided wasn't found: " << param_file << "\n";
      return EXIT_FAILURE;
    }
  }
  else
  {
    auto baseParamFile = "1_params_default.csv";
    auto temp = cbica::normPath(cbica::getExecutablePath() + "../data/" + baseParamFile);
    if (cbica::isFile(temp))
    {
      param_file = temp;
    }
    else
    {
      std::string dataDir = "";
#ifndef APPLE
      dataDir = cbica::normPath(cbica::getExecutablePath() + "/../data/");
#else
      dataDir = cbica::normPath(cbica::getExecutablePath() + "/../Resources/data/";
#endif
      if (cbica::isFile(temp))
      {
        param_file = temp;
      }
      else
      {
        std::cerr << "No default param file was found. Please set it manually using '-p'.\n";
        return EXIT_FAILURE;
      }
    }
  }

  if (debug)
  {
    std::cout << "[DEBUG] Performing dos2unix using CBICA TK function; doesn't do anything in Windows machines.\n";
  }
  cbica::dos2unix(param_file);
  std::cout << "Using param file: " << param_file << "\n";

  if (parser.isPresent("b"))
  {
    parser.getParameterValue("b", multipatient_file);
  }

  if (parser.isPresent("n"))
  {
    parser.getParameterValue("n", patient_id);
  }

  if (multipatient_file.empty() && patient_id.empty())
  {
    std::cerr << "NO INPUT PROVIDED" << "\n" <<
      "Please provide a patient id or path to csv file containing patient details";
    return EXIT_FAILURE;
  }
  if (!multipatient_file.empty() && !patient_id.empty())
  {
    std::cerr << "MULTIPLE INPUT PROVIDED" << "\n" <<
      "Please provide either a patient id or path to csv file containing patient details";
    return EXIT_FAILURE;
  }
  if (parser.isPresent("of"))
  {
    parser.getParameterValue("of", offset_String);
  }

  if (!patient_id.empty())
  {
    std::cout << "Single subject computation selected.\n";

    cbica::replaceString(selected_roi_string, ",", "|");
    cbica::replaceString(roi_labels_string, ",", "|");

    if (debug)
    {
      std::cout << "[DEBUG] Patient ID: " << patient_id << "\n";
      std::cout << "[DEBUG] Images: " << image_path_string << "\n";
      std::cout << "[DEBUG] Modalities: " << modalities_string << "\n";
      std::cout << "[DEBUG] Mask File: " << maskfilename << "\n";
      std::cout << "[DEBUG] ROI Values: " << selected_roi_string << "\n";
      std::cout << "[DEBUG] ROI Labels: " << roi_labels_string << "\n";
    }

    auto genericImageInfo = cbica::ImageInfo(image_paths[0]);
    switch (genericImageInfo.GetImageDimensions())
    {
    case 2:
    {
      using ImageType = itk::Image < float, 2 >;
      algorithmRunner< ImageType >();
      break;
    }
    case 3:
    {
      using ImageType = itk::Image < float, 3 >;
      algorithmRunner< ImageType >();
      break;
    }
    default:
    {
      std::cerr << "Only 2 and 3 dimension images are supported right now.\n";
      return EXIT_FAILURE;
    }
    }
  }
  else // where multi-subject file is passed 
  {
    // cbica::Logging(loggerFile, "Multiple subject computation selected.\n");
    // TBD: use the size of allRows to enable parallel processing, if needed
    std::vector< std::vector < std::string > > allRows; // store the entire data of the CSV file as a vector of columns and rows (vector< rows <cols> >)

    if (debug)
    {
      std::cout << "[DEBUG] Performing dos2unix using CBICA TK function; doesn't do anything in Windows machines.\n";
    }
    cbica::dos2unix(multipatient_file);
    std::ifstream inFile(multipatient_file.c_str());
    std::string csvPath = cbica::getFilenamePath(multipatient_file);

    while (inFile.good())
    {
      std::string line;
      std::getline(inFile, line);
      line.erase(std::remove(line.begin(), line.end(), '"'), line.end());
      if (!line.empty())
      {
        allRows.push_back(cbica::stringSplit(line, ","));
      }
    }
    inFile.close();

    auto maxThreads = omp_get_max_threads();
    if (threads == -1)
    {
      threads = maxThreads;
    }
    else if (threads > maxThreads)
    {
      threads = maxThreads;
    }

#pragma omp parallel for num_threads(threads)
    for (int j = 1; j < static_cast<int>(allRows.size()); j++)
    {
      patient_id.clear();
      image_paths.clear();
      modality_names.clear();
      selected_roi_string.clear();
      roi_labels_string.clear();

      for (size_t k = 0; k < allRows[0].size(); k++)
      {
        auto check_wrap = allRows[0][k];
        std::transform(check_wrap.begin(), check_wrap.end(), check_wrap.begin(), ::tolower);

        if ((check_wrap == "patient_id") || (check_wrap == "patientid") || (check_wrap == "name") || (check_wrap == "patient_name") || (check_wrap == "patientname"))
        {
          patient_id = allRows[j][k];
        }
        if ((check_wrap == "images") || (check_wrap == "inputs") || (check_wrap == "inputimages"))
        {
          std::string parsedImageNames = allRows[j][k];
          image_paths = splitTheString(parsedImageNames);
        }
        if (check_wrap == "modalities")
        {
          std::stringstream modalitynames(allRows[j][k]); std::string parsed;
          while (std::getline(modalitynames, parsed, '|'))
          {
            modality_names.push_back(parsed);
          }
        }
        if ((check_wrap == "roifile") || (check_wrap == "maskfile") || (check_wrap == "roi") || (check_wrap == "mask") || (check_wrap == "segmentation"))
        {
          maskfilename = allRows[j][k];
        }
        if ((check_wrap == "selected_roi") || (check_wrap == "selectedroi"))
        {
          selected_roi_string = allRows[j][k];
          selected_roi = cbica::stringSplit(selected_roi_string, "|");
        }
        if ((check_wrap == "roi_label") || (check_wrap == "roilabel") || (check_wrap == "label") || (check_wrap == "roi_labels") || (check_wrap == "roilabels") || (check_wrap == "labels"))
        {
          roi_labels_string = allRows[j][k];
          roi_labels = cbica::stringSplit(roi_labels_string, "|");
        }
        if ((check_wrap == "outputfile") || (check_wrap == "output") || (check_wrap == "outputdir"))
        {
          outputdir = allRows[j][k];
        }
        //else if (cbica::isDir(outputdir))
        //{
        //  outputdir = outputdir + "/" + patient_id + ".csv";
        //}
      } // end of k-loop

      if (debug)
      {
        std::cout << "[DEBUG] Patient ID: " << patient_id << "\n";
        std::cout << "[DEBUG] Images: " << image_paths[0];
        std::string temp = modality_names[0];
        if (image_paths.size() != modality_names.size())
        {
          std::cerr << "Number of images and modalites do not match.\n";
          exit(EXIT_FAILURE);
        }
        for (size_t imageNum = 1; imageNum < image_paths.size(); imageNum++)
        {
          std::cout << "," + image_paths[imageNum];
          temp += "," + modality_names[imageNum];
        }
        std::cout << "\n";
        std::cout << "[DEBUG] Modalities: " << temp << "\n";
        std::cout << "[DEBUG] Mask File: " << maskfilename << "\n";
        std::cout << "[DEBUG] ROI Values: " << selected_roi_string << "\n";
        std::cout << "[DEBUG] ROI Labels: " << roi_labels_string << "\n";
      }
      // cbica::Logging(loggerFile, "Starting computation for subject '" + std::string(j) + "' of '" + std::string(allRows.size() - 1) + "' total subjects.\n");
      //std::cout << "Starting computation for subject '" << j << "' of '" << allRows.size() - 1 << "' total subjects.\n";

      // ensuring different dimensions are handled properly
      auto genericImageInfo = cbica::ImageInfo(image_paths[0]);
      switch (genericImageInfo.GetImageDimensions())
      {
      case 2:
      {
        using ImageType = itk::Image < double, 2 >;
        algorithmRunner< ImageType >();
        break;
      }
      case 3:
      {
        using ImageType = itk::Image < float, 3 >;
        algorithmRunner< ImageType >();
        break;
      }
      default:
      {
        // cbica::Logging(loggerFile, "Only 2 and 3 dimension images are supported right now.\n");
        //return EXIT_FAILURE;
      }
      }
    } // end of j-loop

  }

  std::cout << "Finished.\n";

  return EXIT_SUCCESS;
}
