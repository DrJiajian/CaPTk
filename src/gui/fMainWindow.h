/**
\file  fMainWindow.h

\brief Declaration of fMainWindow class

http://www.med.upenn.edu/sbia/software/ <br>
software@cbica.upenn.edu

Copyright (c) 2018 University of Pennsylvania. All rights reserved. <br>
See COPYING file or https://www.med.upenn.edu/sbia/software-agreement.html

*/

#pragma once


#ifndef _fMainWindow_h_
#define _fMainWindow_h_


#include "CAPTk.h"
#include "Slicer.h"
#include "SlicerManager.h"
#include "NiftiDataManager.h"
#include "EGFRvIIISurrogateIndex.h"
#include "RecurrenceEstimator.h"
#include "PseudoProgressionEstimator.h"
#include "SurvivalPredictor.h"
#include "EGFRvIIIIndexPredictor.h"
#include "ui_fMainWindow.h"
#include "InteractorStyleNavigator.h"
#include "QTablePushButton.h"
#include "SimpleImageManager.h"
#include "OutputWritingManager.h"
#include "PreprocessingPipelineClass.h"
#include "FeatureExtractionClass.h"
#include "FeatureReductionClass.h"
#include "FeatureScalingClass.h"
#include "GeodesicSegmentation.h"
#include "fHelpDialog.h"
#include "fHelpTutorial.h"
#include "PopulationAtlases.h"
#include "ImagingSubtypePredictor.h"
#include "MolecularSubtypePredictor.h"
#include "FetalBrain.h"
#include "TrainingModule.h"
//#include "GreedyParameters.h"
//#include "GreedyAPI.h"
//#include "lddmm_data.h"
//#include "lddmm_common.h"
//#include "GeodesicTrain.h"

#define USE_PROCESSDIALOG

enum TAB_TYPE
{
  TAB_IMAGES, TAB_TUMOR, TAB_DRAW, TAB_FEWATURE
};

typedef itk::Image< short, 3 > GenericImage;

// multiLabel
enum DRAW_MODE
{
  DRAW_MODE_LABEL_1 = 1, DRAW_MODE_LABEL_2, DRAW_MODE_LABEL_3, DRAW_MODE_LABEL_4, DRAW_MODE_LABEL_5, DRAW_MODE_LABEL_6, 
  DRAW_MODE_LABEL_7, DRAW_MODE_LABEL_8, DRAW_MODE_LABEL_9
};

enum SHAPE_MODE
{
  SHAPE_MODE_NONE = 0, SHAPE_MODE_ERASER, SHAPE_MODE_FREE_HAND, SHAPE_MODE_LINE, SHAPE_MODE_RECTANGLE, SHAPE_MODE_CIRCLE, SHAPE_MODE_FILL
};

//TBD move all this to util class 
template <typename ITK_Exporter, typename VTK_Importer>
void ConnectPipelines(ITK_Exporter &exporter, VTK_Importer* importer)
{
  importer->SetUpdateInformationCallback(exporter->GetUpdateInformationCallback());
  importer->SetPipelineModifiedCallback(exporter->GetPipelineModifiedCallback());
  importer->SetWholeExtentCallback(exporter->GetWholeExtentCallback());
  importer->SetSpacingCallback(exporter->GetSpacingCallback());
  importer->SetOriginCallback(exporter->GetOriginCallback());
  importer->SetScalarTypeCallback(exporter->GetScalarTypeCallback());
  importer->SetNumberOfComponentsCallback(exporter->GetNumberOfComponentsCallback());
  importer->SetPropagateUpdateExtentCallback(exporter->GetPropagateUpdateExtentCallback());
  importer->SetUpdateDataCallback(exporter->GetUpdateDataCallback());
  importer->SetDataExtentCallback(exporter->GetDataExtentCallback());
  importer->SetBufferPointerCallback(exporter->GetBufferPointerCallback());
  importer->SetCallbackUserData(exporter->GetCallbackUserData());
}
template <typename VTK_Exporter, typename ITK_Importer>
void ConnectPipelines(VTK_Exporter* exporter, ITK_Importer &importer)
{
  importer->SetUpdateInformationCallback(exporter->GetUpdateInformationCallback());
  importer->SetPipelineModifiedCallback(exporter->GetPipelineModifiedCallback());
  importer->SetWholeExtentCallback(exporter->GetWholeExtentCallback());
  importer->SetSpacingCallback(exporter->GetSpacingCallback());
  importer->SetOriginCallback(exporter->GetOriginCallback());
  importer->SetScalarTypeCallback(exporter->GetScalarTypeCallback());
  importer->SetNumberOfComponentsCallback(exporter->GetNumberOfComponentsCallback());
  importer->SetPropagateUpdateExtentCallback(exporter->GetPropagateUpdateExtentCallback());
  importer->SetUpdateDataCallback(exporter->GetUpdateDataCallback());
  importer->SetDataExtentCallback(exporter->GetDataExtentCallback());
  importer->SetBufferPointerCallback(exporter->GetBufferPointerCallback());
  importer->SetCallbackUserData(exporter->GetCallbackUserData());
}

//! Converts a VTK-Image to ITK-Image on the basis of the pixel type definition of the former and latter
template<class InputPixelType, class OutputPixelType, int VImageDimension>
typename itk::Image<OutputPixelType, VImageDimension>::Pointer convertVtkToItk(vtkSmartPointer<vtkImageData> vtkInput)
{
  typedef itk::Image<InputPixelType, VImageDimension> InputImageType;
  typedef itk::Image<OutputPixelType, VImageDimension> OutputImageType;
  typedef itk::VTKImageImport<InputImageType> InputImageImportType;

  vtkSmartPointer<vtkImageExport> inputImageExporter = vtkImageExport::New();
#if VTK_MAJOR_VERSION <= 5
  inputImageExporter->SetInput(vtkInput);
#else
  inputImageExporter->SetInputData(vtkInput);
#endif
  typename InputImageImportType::Pointer inputImageImporter = InputImageImportType::New();

  ConnectPipelines(inputImageExporter.GetPointer(), inputImageImporter);

  typename InputImageType::Pointer inputImage = const_cast<InputImageType*>(inputImageImporter->GetOutput());
  inputImage->Update();
  if (inputImage.IsNull()) {
    std::cerr << "vtkInput is invalid." << std::endl;
  }

  typedef itk::CastImageFilter< InputImageType, OutputImageType > CastFilterType;
  typename CastFilterType::Pointer castFilter = CastFilterType::New();
  castFilter->SetInput(inputImage);
  castFilter->Update();

  typename OutputImageType::Pointer outputImage = castFilter->GetOutput();
  outputImage->Update();
  if (outputImage.IsNull()) {
    std::cerr << "inputImage is invalid." << std::endl;
    return NULL;
  }

  return outputImage;
}

//! Converts a VTK-Image to ITK-Image on the basis of the pixel type definition of the latter
template<class OutputPixelType, int VImageDimension>
typename itk::Image<OutputPixelType, VImageDimension>::Pointer convertVtkToItk(vtkSmartPointer<vtkImageData> vtkInput) //TBD move to cbica util 
{
  std::string InputPixelType = vtkInput->GetScalarTypeAsString();
  if (InputPixelType == "short")
  {
    return convertVtkToItk<short, OutputPixelType, VImageDimension>(vtkInput);
  }
  else if (InputPixelType == "unsigned short")
  {
    return convertVtkToItk<unsigned short, OutputPixelType, VImageDimension>(vtkInput);
  }
  else if (InputPixelType == "char")
  {
    return convertVtkToItk<char, OutputPixelType, VImageDimension>(vtkInput);
  }
  else if (InputPixelType == "unsigned char")
  {
    return convertVtkToItk<unsigned char, OutputPixelType, VImageDimension>(vtkInput);
  }
  else if (InputPixelType == "int")
  {
    return convertVtkToItk<int, OutputPixelType, VImageDimension>(vtkInput);
  }
  else if (InputPixelType == "unsigned int")
  {
    return convertVtkToItk<unsigned int, OutputPixelType, VImageDimension>(vtkInput);
  }
  else if (InputPixelType == "double")
  {
    return convertVtkToItk<double, OutputPixelType, VImageDimension>(vtkInput);
  }
  else if (InputPixelType == "float")
  {
    return convertVtkToItk<float, OutputPixelType, VImageDimension>(vtkInput);
  }
  else {
    std::cerr << "Error, input pixel type : " << InputPixelType << " unknown !" << std::endl;
  }
  return NULL;
}

//template <unsigned int VDim, typename TReal>
//class GreedyRunner
//{
//public:
//    static int Run(GreedyParameters &param)
//    {
//        GreedyApproach<VDim, TReal> greedy;
//        return greedy.Run(param);
//    }
//};

/**
\class fMainWindow

\brief This is the main UI class for CaPTk
*/
#if __GNUC__
#pragma GCC visibility push(hidden)
#endif
class fMainWindow : public QMainWindow, private Ui::fMainWindow
{
  Q_OBJECT

public:
  //! Default constructor
  fMainWindow();

  //! Default destructor
  ~fMainWindow();
  
  QTableWidget * m_imagesTable;
  QTableWidget * m_nonVisImagesTable;

  //! Check if a valid mask is defined for the current instance of the 
  bool isMaskDefined()
  {
    if (!mSlicerManagers.empty())
    {
      auto minMaxComputer = itk::MinimumMaximumImageCalculator<ImageTypeFloat3D>::New();
      minMaxComputer->SetImage(convertVtkToItk<float, 3>(mSlicerManagers[0]->mMask));
      minMaxComputer->Compute();
      if (minMaxComputer->GetMaximum() > 0)
      {
        return true;
      }
    }

    return false;
  }

  /*
  \brief Change direction cosine of image to identity
  \param image The image whose direction is altered
  */
  template< class TImageType = ImageTypeFloat3D >
  typename TImageType::Pointer ChangeImageDirectionToIdentity(const typename TImageType::Pointer image)
  {
    auto infoChangeFilter = itk::ChangeInformationImageFilter< TImageType >::New();
    typename TImageType::DirectionType direction_new;
    direction_new.SetIdentity();
    infoChangeFilter->SetInput(image);
    infoChangeFilter->SetOutputDirection(direction_new);
    infoChangeFilter->ChangeDirectionOn();
    infoChangeFilter->Update();
    return infoChangeFilter->GetOutput();
  }

  /**
  \brief Load images into memory
  
  \param filenames Vector of image filenames which need to be loaded (comes from "Load" dialog)
  \param imagetype_int Either NIfTI or DICOM
  \param bSkipDup Skip duplicates, defaults to true
  */
  void LoadSlicerImages(const std::string &fileName, const int &imagetype_int, bool bSkipDup = true);


  /**
  \brief Load non-viewing images into memory
  
  \param directoryname Directory name in which the non-viewing images are present
  \param imagetype_int Either NIfTI or DICOM
  \param imagesubtype Modality of the image (T1, T2, ...)
  */
  void LoadNonViewingImages(const std::string &directoryname, const int &imagetype_int, const int &imagesubtype);

  /**
  \brief Initialize drawing mask on the image
  */
  void InitMask(vtkImageData* image);



  /**
  \brief Initializer for the entire rendering pipeline
  */
  void InitDisplay();

  /**
  \brief Initializes difference slicer views of a single image
  */
  void InitSlicers();

  /**
  \brief Render the sliders to adjust the view of the image slices.

  Initializes it at the middle of the whole image.

  \param slicer If many images are present, this keeps a count of which image is being rendered
  \param window The window number (goes from 1 to 3)
  */
  void DisplaySliders(int slicer, int window);

  /**
  \brief Get image index from the visualization table
  */
  int GetSlicerIndexFromItem(QTableWidgetItem* item);

  /**
  \brief Get the corresponding table item from the image
  */
  QTableWidgetItem* GetItemFromSlicerManager(SlicerManager* sm);
  
  /**
  \brief Construct near and far indeces from the initialized mask
  */

  template <class TImageType>
  typename TImageType::Pointer GetImageWithLabels(std::vector<double> labels, typename TImageType::Pointer inputimage)
  {
    auto img = convertVtkToItk<float, 3>(mSlicerManagers[0]->mMask);

    typename TImageType::Pointer output = TImageType::New();
    output->CopyInformation(img);
    output->SetRequestedRegion(img->GetLargestPossibleRegion());
    output->SetBufferedRegion(img->GetBufferedRegion());
    output->Allocate();
    output->FillBuffer(0);

    typedef itk::ImageRegionIteratorWithIndex< TImageType > IteratorType;
    IteratorType maskIt(img, img->GetLargestPossibleRegion());
    IteratorType outputIt(output, output->GetLargestPossibleRegion());
    maskIt.GoToBegin();
    outputIt.GoToBegin();
    while (!maskIt.IsAtEnd())
    {
      for (int j = 0; j < labels.size(); j++)
      {
        if (maskIt.Get() == labels[j])
        {
          outputIt.Set(VOXEL_STATUS::ON);
        }
      }
      ++maskIt;
	  ++outputIt;
    }
    return output;
  }

  template<class TImageType>
  void FormulateNearFarPoints(std::vector<typename TImageType::IndexType> &nearIndices, std::vector<typename TImageType::IndexType> &farIndices)
  {
    if (mSlicerManagers.size() <= 0)
      return;

    typename TImageType::Pointer img = convertVtkToItk<typename TImageType::PixelType, 3>(mSlicerManagers[0]->mMask);
    itk::ImageRegionIteratorWithIndex <TImageType> maskIt(img, img->GetLargestPossibleRegion());
    maskIt.GoToBegin();
    while (!maskIt.IsAtEnd())
    {
      if (maskIt.Get() == 1)
      {
        nearIndices.push_back(maskIt.GetIndex());
      }
      else if (maskIt.Get() == 2)
        farIndices.push_back(maskIt.GetIndex());
      ++maskIt;
    }
  }

  /**
  \brief Rescales image intensity in the range of 0-255
  */
  ImageTypeFloat3D::Pointer RescaleImageIntensity(ImageTypeFloat3D::Pointer image);

  /*
  \brief Drag event initialization
  */
  void dragEnterEvent(QDragEnterEvent *event);

  /*
  \brief Drop Event parsing
  */
  void dropEvent(QDropEvent *event);

  /*
  \brief For fast developer testing .TBD:  remove or disable.
  */
  void loadFromCommandLine(std::vector< QString > files, const std::string &maskImage = "", 
    const std::string &tumorPointFile = "", const std::string &tissuePointFile = "", bool firstRun = false)
  {
    auto qvectorString = QVector< QString >::fromStdVector(files);
    auto lst = QStringList::fromVector(QVector< QString >::fromStdVector(files));
    this->openImages(lst, true);
    if (!maskImage.empty())
    {
      this->readMaskFile(maskImage);
    }
    if (!tumorPointFile.empty())
    {
      this->tumorPanel->sLoad(tumorPointFile.c_str());
    }
    if (!tissuePointFile.empty())
    {
      this->tumorPanel->tLoad(tissuePointFile.c_str());
    }

#ifdef CAPTK_PACKAGE_PROJECT
    if (firstRun)
    {
      this->CloseAllImages();
    }
#endif
  }

signals:
  void SelectedImageHasChanged(SlicerManager *);
  void LandmarksFocused(bool bFocused);
  void SeedPointsFocused(bool bFocused);
  void TissuePointsFocused(bool bFocused);

  public slots:
  /**
  \brief Updates draw mode when drawing panel changes
  */
  void updateDrawMode(int mode = -1);

  /**
  \brief Updates number of near and far points in the table
  */
  void UpdateNumberOfPointsInTable();

  /**
  \check whether all the ROIs are present for recurrence estimation or not
  */
  //bool CheckSeedPointsForRecurrence();

  /**
  \brief Initializes preset combo-box with the default options
  */
  void SetPresetComboBox();

  /**
  \brief Main function that performs EGFRvIII estimation on the displayed image
  */
  void StartEGFREstimate();

  /**
  \brief get the mask image 
  */
 ImageTypeFloat3D::Pointer getMaskImage();


 /**
 \brief function that performs segmentation and UI measurements of Featl Ventriculomegaly application
 */
 void skullstripfunc();
 void Predict();
 void TrainNewFetalModel(const std::string &directory, const std::string &outputdirectory);

  /**
  \brief get images loaded and their file names
  */
 std::vector<ImageTypeFloat3D::Pointer> getLodedImages(std::vector<std::string> &fileNames, std::vector<std::string> &modality, bool onlySelected = false);

  /**
  \brief Main function that estimates recurrence on the displayed test subject
  param outputdirectory The directory where recurrence map will be written
  param cbT1Data Whether T1 data is present or not
  param cbT1ceData Whether T1CE data is present or not
  param cbT2Data Whether T2 data is present or not
  param cbT2FlairData Whether T2-Flair data is present or not
  param cbDTIData Whether DTI data is present or not
  param cbPerfData Whether Perfusion data is present or not
  param cbDistData Whether Distance feature need to be used or not
  */
  void StartRecurrenceEstimate(const std::string &outputdirectory,  bool cbT1Data,  bool cbDTIData,  bool cbPerfData,  bool cbDistData);
  void LoadedSubjectExistingRecurrenceEstimate(const std::string &outputdirectory, const std::string &modeldirectory, bool cbT1Data, bool cbDTIData, bool cbPerfData, bool cbDistData);
  //void StartSurvivalEstimate(const std::string output, const std::string model, double age);
  /**
  \brief Main function that estimates recurrence on test subjects by using an existing model
  param modeldirectory The directory where model related files are stored
  param inputdirectory The directory where test data is stored
  param outputdirectory The directory where output data will be stored

  param cbT1Data Whether T1 data is present or not
  param cbT1ceData Whether T1CE data is present or not
  param cbT2Data Whether T2 data is present or not
  param cbT2FlairData Whether T2-Flair data is present or not
  param cbDTIData Whether DTI data is present or not
  param cbPerfData Whether Perfusion data is present or not
  param cbDistData Whether Distance feature need to be used or not
  */
  void RecurrenceEstimateOnExistingModel(const std::string &modeldirectory, const std::string &inputdirectory, const std::string &outputdirectory, bool cbConventionalData,  bool cbDTIData,  bool cbPerfData,  bool cbDistData);

  /**
  \brief Main function that trains a model on many training subjects
  param directory The directory that contains training data
  param outputdirectory The directory where trained model related data will be stored
  param cbT1Data Whether T1 data is present or not
  param cbT1ceData Whether T1CE data is present or not
  param cbT2Data Whether T2 data is present or not
  param cbT2FlairData Whether T2-Flair data is present or not
  param cbDTIData Whether DTI data is present or not
  param cbPerfData Whether Perfusion data is present or not
  param cbDistData Whether Distance feature need to be used or not
  */
  void TrainNewModelOnGivenData(const std::string &directory, const std::string &outputdirectory,  bool cbConvData,  bool cbDTIData,  bool cbPerfData,  bool cbDistData);
















  /**
  \brief Main function that estimates pseudoprogression on the displayed test subject
  param outputdirectory The directory where recurrence map will be written
  param cbT1Data Whether T1 data is present or not
  param cbT1ceData Whether T1CE data is present or not
  param cbT2Data Whether T2 data is present or not
  param cbT2FlairData Whether T2-Flair data is present or not
  param cbDTIData Whether DTI data is present or not
  param cbPerfData Whether Perfusion data is present or not
  param cbDistData Whether Distance feature need to be used or not
  */
  void LoadedSubjectExistingPseudoprogressionEstimate(const std::string &outputdirectory, const std::string &modeldirectory, bool cbT1Data, bool cbDTIData, bool cbPerfData, bool cbDistData);
  //void StartSurvivalEstimate(const std::string output, const std::string model, double age);
  /**
  \brief Main function that estimates PseudoProgression on test subjects by using an existing model
  param modeldirectory The directory where model related files are stored
  param inputdirectory The directory where test data is stored
  param outputdirectory The directory where output data will be stored

  param cbT1Data Whether T1 data is present or not
  param cbT1ceData Whether T1CE data is present or not
  param cbT2Data Whether T2 data is present or not
  param cbT2FlairData Whether T2-Flair data is present or not
  param cbDTIData Whether DTI data is present or not
  param cbPerfData Whether Perfusion data is present or not
  param cbDistData Whether Distance feature need to be used or not
  */
  void PseudoprogressionEstimateOnExistingModel(const std::string &modeldirectory, const std::string &inputdirectory, const std::string &outputdirectory, bool cbConventionalData, bool cbDTIData, bool cbPerfData, bool cbDistData);

  /**
  \brief Main function that trains a model on many training subjects
  param directory The directory that contains training data
  param outputdirectory The directory where trained model related data will be stored
  param cbT1Data Whether T1 data is present or not
  param cbT1ceData Whether T1CE data is present or not
  param cbT2Data Whether T2 data is present or not
  param cbT2FlairData Whether T2-Flair data is present or not
  param cbDTIData Whether DTI data is present or not
  param cbPerfData Whether Perfusion data is present or not
  param cbDistData Whether Distance feature need to be used or not
  */
  void TrainNewPseudoprogressionModelOnGivenData(const std::string &directory, const std::string &outputdirectory, bool cbConvData, bool cbDTIData, bool cbPerfData, bool cbDistData);





  /**
  \brief Survival analysis using Existing model

  \param modeldirectory The directory where the existing model files are stored
  \param inputdirectory The input subject directory (pre-sorted)
  \param outputdirectory The output directory to save the data
  */
  void CallForSurvivalPredictionOnExistingModelFromMain(const std::string modeldirectory, const std::string inputdirectory, const std::string outputdirectory);

  /**
  \brief Create new model for Survival analysis 

  \param inputdirectory The input subjects directory (pre-sorted)
  \param outputdirectory The output directory to save the data
  */
  void CallForNewSurvivalPredictionModelFromMain(const std::string inputdirectory, const std::string outputdirectory);


  /**
  \brief EGFRvIII estimation using Existing model

  \param modeldirectory The directory where the existing model files are stored
  \param inputdirectory The input subject directory (pre-sorted)
  \param outputdirectory The output directory to save the data
  */
  void CallForEGFRvIIIPredictionOnExistingModelFromMain(const std::string modeldirectory, const std::string inputdirectory, const std::string outputdirectory);

  /**
  \brief Create new model for EGFRvIII prediction

  \param inputdirectory The input subjects directory (pre-sorted)
  \param outputdirectory The output directory to save the data
  */
  void CallForNewEGFRvIIIPredictionModelFromMain(const std::string inputdirectory, const std::string outputdirectory);

  
  /**
  \brief MolecularSubtype using Existing model

  \param modeldirectory The directory where the existing model files are stored
  \param inputdirectory The input subject directory (pre-sorted)
  \param outputdirectory The output directory to save the data
  */
  void CallForMolecularSubtypePredictionOnExistingModelFromMain(const std::string modeldirectory, const std::string inputdirectory, const std::string outputdirectory);

  /**
  \brief Create new model for MolecularSubtype

  \param inputdirectory The input subjects directory (pre-sorted)
  \param outputdirectory The output directory to save the data
  */
  void CallForNewMolecularSubtypePredictionModelFromMain(const std::string inputdirectory, const std::string outputdirectory);

  /**
  \brief Call the Skull Stripping module from ITK

  \param referenceAtlas The reference atlas (defaults to SRI24 atlas)
  \param referenceMask The reference mask (defaults to SRI24 mask)
  \param inputImageFile The input image file
  \param outputImageFile The output file to save 
  */
  void CallImageSkullStripping(const std::string referenceAtlas, const std::string referenceMask, const std::string inputImageFile, const std::string outputImageFile);
  void CallPCACalculation(const int, const std::string outputFolder);
  void CallPerfusionMeasuresCalculation(const double TE, const bool rcbv, const bool psr, const bool ph, const std::string inputfile, const std::string outputFolder);
  void CallDiffusionMeasuresCalculation(const std::string inputImage, const std::string maskImage, const std::string BValFile, const std::string BVecFile, const bool ax, const bool fa, const bool rad, const bool tr, const std::string outputFolder);
  void CallTrainingSimulation(const std::string featuresfile, const std::string targetfile, const std::string outputFolder,int,int);

  /**
  \brief Call DCM2NII for DICOM conversion and load the image into CaPTk

  \param firstImageInSeries First image in the DICOM series (assumes the folder has a single DICOM series)
  \param outputFileName The output NIfTI file
  */
  void CallDCM2NIfTIConversion(const std::string firstImageInSeries, bool loadAsImage);

  /**
  \brief Call DCM2NII for DICOM conversion

  \param firstImageInSeries First image in the DICOM series (assumes the folder has a single DICOM series)
  \param outputFileName The output NIfTI file
  */
  void CallDCM2NIfTIConversion(const std::string firstImageInSeries, const std::string outputFileName);

  /**
  \brief Call the Deep Medic Segmentation dialog
  */
  void CallDeepMedicSegmentation(const std::string outputDirectory);

  /**
  \brief Call Histogram Matching module of ITK
  */
  void CallImageHistogramMatching(const std::string referenceImage, const std::string inputImageFile, const std::string outputImageFile);

  /**
  \brief Call Deep Medic Normalization module
  */
  void CallImageDeepMedicNormalizer(const std::string inputImage, const std::string maskImage, const std::string outputImageFile, 
    const std::string quantLower, const std::string quantUpper,
    const std::string cutoffLower, const std::string cutoffUpper, bool wholeImageMeanThreshold);

  //! Call the Directionality Estimator
  void CallDirectionalityEstimator(const std::string roi1File, const std::string roi2File, const std::string outputDir);

  void CallWhiteStripe(double twsWidth, int sliceStartZ, int sliceStopZ, int tissuesMax, double smoothMax, double smoothDelta, int histSize,
    bool T1Image, const std::string outputFileName);

  /**
  \brief Generate population atlas
  */
  void CallGeneratePopualtionAtlas(const std::string inputdirectory, const std::string inputlabel, const std::string inputatlas, const std::string outputImageFile);
  
  /**
\brief Generete SBRT Nodule
*/
  void CallSBRTNodule(const std::string seedImage, const int labelValue);

  /**
  \brief Function that updates the co-ordinates (X and Y) of border
  param startX starting X co-ordinate
  param startY starting Y co-ordinate
  param endX ending X co-ordinate
  param endY ending Y co-ordinate
  */
  void UpdateBorderWidget(double startX, double startY, double endX, double endY);

  /**
  \brief Function that updates the co-ordordinatesicates (Z) of border
  param startX starting Z co-ordinate
  param startY starting Z co-ordinate
  */
  void UpdateBorderWidget(double startZ, double endZ);

  /**
  \brief Adds actions in the action log
  param points The voxels corresponding to action
  */
  void UpdateActionQ(const QVariantList& list)
  {
	  std::vector< PointVal > points;
	  for (int i = 0; i < list.size(); i++)
	  {
		  points.push_back(qvariant_cast<PointVal>(list[i]));
	  }
	  UpdateAction(points);
  }
  void UpdateAction(std::vector< PointVal > points = std::vector< PointVal >());

  /**
  \brief Brief description about the software
  */
  void about();

  /**
  \brief Help for interactions
  */
  void help_Interactions();

  //! Don't show tutorial again
  void skipTutorial(bool flag)
  {
    m_skipTutorialOnNextRun = flag;
  }

  /**
  \brief Help for downloading Sample Data
  */
  void help_Download(QAction* action)
  {
    auto currentApp = action->text().toStdString();
    std::string path = getCaPTkDataDir();
    std::string link =
#ifdef _WIN32
      path + "/GnuWin32/bin/wget.exe"
#else
      "wget"
#endif
      + std::string(" ftp://www.nitrc.org/home/groups/captk/downloads/SampleData_1.6.0/") + currentApp + ".zip" +
      " -O " + captk_SampleDataFolder + "/" + currentApp + ".zip";

    cbica::Logging(loggerFile, link);

    ShowErrorMessage("Starting download, may take a while, depending on your net bandwidth", this, "Downloading...");

    if (std::system((link).c_str()) != 0)
    {
      ShowErrorMessage("CaPTk couldn't open the browser to download specified sample data.");
      return;
    }
    else
    {
      std::string dataMessage = "Data has been saved to: " + captk_SampleDataFolder;
      ShowMessage(dataMessage, "Saved");
      return;
    }
  }
  void help_Discussion();
  void help_Downloads();
  void help_HelpForum();
  void help_BugTracker();
  void help_FeatureRequests();

  void help_contextual(const std::string startPage)
  {
    mHelpDlg->setNewStartPage(startPage);
    mHelpDlg->show();
  }

  /**
  \brief Open Nifti image functionality. Shows dialog to select nifti files
  */
  void openImages(QStringList files = QStringList(), bool callingFromCmd = false);

  /**
  \brief Function called when the sliders of axial view is changed
  */
  void AxialViewSliderChanged();
  
  /**
  \brief Function called when the sliders of coronal view is changed
  */
  void CoronalViewSliderChanged();
  
  /**
  \brief Function called when the sliders of saggital view is changed
  */
  void SaggitalViewSliderChanged();
  
  /**
  \brief Closing viewing image by pressing X in front of the image
  param item The current selected item of the table
  */
  void CloseImage(QTableWidgetItem* item);

  /**
  \brief Closing non-viewing image by pressing X in front of the image
  param item The current selected item of the table
  */
  void CloseNonViewingDTIImage(QTableWidgetItem* item);


  void clearMask(int label=-1);

  void makeStroke(std::vector<itk::Image<short, 3>::IndexType>& indices, const int value);
  
  int getSelectedDrawLabel()
  {
    return drawingPanel->getSelectedDrawLabel();
  }
  int getSelectedDrawSize()
  {
    return drawingPanel->getSelectedDrawSize();
  }
  
  /**
  \brief Save near/far drawing in Nifti format
  */
  void SaveDrawing();
  
  /**
  \brief Save near/far drawing in DICOM format
  */
  void SaveDicomDrawing();
  
  /**
  \brief Save initial seed drawing in Nifti format
  */
  void SaveSeedDrawing();

  ///**
  //\brief Load seed drawing from a Nifti file
  //*/
  //void LoadSeedDrawing();

  /**
  \brief Load annotated ROI from qt file accept box
  */
  void LoadDrawing();

  /**
  \brief Load annotated ROI from filename
  */
  void LoadDrawing(const std::string &maskFile);

  ///**
  //\brief Load near/far drawing from a DICOM file
  //*/
  //void LoadDicomDrawing();

  /**
  \brief Combines near and far drawn points in one long vector to be used for edema segmentation based on region growing
  */
  VectorVectorDouble FormulateDrawingPointsForEdemaSegmentation();

  /**
  \brief Puts initial seed points in one vector to be used for tumor segmentation 
  */
  VectorVectorDouble FormulateDrawingPointsForTumorSegmentation();



  /**
  \brief Save the current selected Nifti image
  */
  void SaveImage();
  
  /**
  \brief Save the current selected DICOM image
  */
  void SaveDicomImage();

  /**
  \brief Get indices of particular label
  */
  VectorVectorDouble GetMaskLabelIndices(const int label);
  
  /**
  \brief Read ROI from a file into the memory
  */
  void readMaskFile(const std::string &maskFileName);

  /**
  \brief Change the size of drawing/erasing brush
  */
  void ChangeBrushSize(int size);

  /**
  \brief Changes the label in the drawing
  */
  void ChangeDrawingLabel(int drawingLabel); // multiLabel uncomment this function

  /**
  \brief Changes the opacity in the drawing
  */
  void ChangeMaskOpacity(int newMaskOpacity); // multiLabel uncomment this function

  /**
  \brief Checks whether required images are present for the recurrence estimation application
  */
  bool CheckCompletenessOfInputData(bool & convDataPresent, bool & perfusionDataPresent, bool & dtiDataPresent, bool existingmodel);


  /**
  \brief Checks whether required images are present for the EGFRvIII estimation application
  */
  bool CheckCompletenessOfInputDataForEGFR(bool &t1ceDataPresent, bool &t2flairDataPresent, bool &perfusionDataPresent);

  /**
  \brief Sets current tissue type to be drawn
  */
  void SetActiveLandmarksType(int type, int row, int col);

  /**
  \brief on change of panel
  */
  void panelChanged(int current);

  void propogateSlicerPosition(int slicerId =0,int imageId=-1);


  /**
  \brief Sets the current selected image
  \param id The id of the current selected image
  */
  void CurrentImageChanged(std::string &id);

  /**
  \brief Updates the information in the bottom panel based on the current selected image
  */
  void ImageInfoChanged();

  /**
  \brief Close the current selected image
  */
  void CloseImage();

  /**
  \brief Close all loaded images
  */
  void CloseAllImages();


  
  /**
  \brief Reset the number of points in the table when all the images are closed
  */
  void ResetNumberOfPoints();

  /**
  \brief This function deals with undo  
  */
  void UndoFunctionality();

  /*
  \brief Fill button logic
  */
  void FillLabel(int label);

  /**
  \brief Sets the opacity of the mask for the displayed images
  */
  void SetOpacity();

  /**
  \brief This function deals with the functionality associated with the change in the overlay slider
  */
  void overlaySliderChanged(int value);

  /**
  \brief This function deals with the functionality associated with the change in the overlay slider
  */
  void imageModalityChanged(int value);

  /**
  \brief This function deals with the functionality associated with the change in the image slider for the perfusion image
  */
  void imageSliderChanged();

  /**
  \brief Resets the transfrmation of each slicer to identity, resets the camera and renders the slicer again
  */
  void ResetTransformationToIdentity();

  /**
  \brief Renders all the images again
  */
  void UpdateRenderWindows();

  /**
  \brief Enable or disable overlay
  \param State of the overlay to set (true/false)
  */
  void overlayUseStateChanged(int state);

  /**
  \brief Passes the item, whose overlay needs to be update, to overlayChanged(QTableWidgetItem *item) function
  */
  void overlayChanged();

  /**
  \brief Sets the item selected in overlay table as overlay on item selected in images table
  */
  void overlayChanged(QTableWidgetItem *item);

  /**
  \brief Maintains record of currentlty displayed image
  */
  void CurrentPickedImageChanged(std::string id);

  /**
  \brief Displays the image currently selected in images table. Internally calls DisplayChanged(QTableWidgetItem *item) function
  */
  void DisplayChanged();

  /**
  \brief Displays the image (item) currently selected in images table
  */
  void DisplayChanged(QTableWidgetItem *item);

  /**
  \brief Updates co-ordinates based on each change in the mouse position
  \param visibity This param shows whether coordinates need to be displayed or not
  \param x LPS/RAS co-ordinate X
  \param y LPS/RAS co-ordinate Y
  \param z LPS/RAS co-ordinate Z
  \param X pixel co-ordiante X
  \param Y pixel co-ordiante Y
  \param Z pixel co-ordiante Z
  \param value Intensity value at current voxel
  */
  void MousePositionChanged(int visibility, double x, double y, double z, double X, double Y, double Z, double value);


  /**
  \brief Sets the window and level values. Calls UpdateWindowLevel function internally
  */
  void WindowLevelChanged();

  /**
  \brief Sets the window and level values. Calls UpdateWindowLevel function internally
  */
  void WindowLevelEdited();
  /**
  \brief Sets the window and level values. Calls UpdateWindowLevel function internally
  */
  void SetWindowLevel(double w, double l);
  /**
  \brief Applies the window and level values selected from spin boxes on currently displayed images
  */
  void UpdateWindowLevel();

  /**
  \brief Applies the value of threshold slider on the displayed images
  */
  void thresholdSpinBoxChanged();


  /**
  \brief Enable mask thresholding using the radio button
  */
  void EnableThresholdOfMask();

  /**
  \brief Creates a link between two currently displayed images
  \param image1 ID of the first image
  \param image2 ID of the second image
  */
  void AddLink(const std::string &image1, const std::string &image2);

  /**
  \brief Removes link between two images
  \param image1 ID of the first image
  \param image2 ID of the second image
  */
  void RemoveLink(const std::string &image1, const std::string &image2);
  /**
  \brief Sets the value of the axial, coronal or sagital sliders
  \param slicer Number of the slicer
  \param slice Value to bet set on the slider
  */
  void UpdateSlice(int slicer, int slice);
  /**
  \brief Sets the minimum and maximum values of axial, coronal or sagital sliders
  \param slicer Number of the slicer
  \param min Minimum value of the slider
  \param max Maximum value of the slider
  */
  void UpdateSliceRange(int slicer, int min, int max);

  /**
  \brief Moves the cursor on the given co-ordinates
  */
  void MoveSlicerCursor(double x, double y, double z, int mode = 0);


  std::vector<std::map<ImageModalityType, std::string>> LoadQualifiedSubjectsFromGivenDirectoryForSurvival (const std::string directoryname);
  std::vector<std::map<ImageModalityType, std::string>> LoadQualifiedSubjectsFromGivenDirectoryForPseudoProgression(const MachineLearningApplicationSubtype type, const std::string &directoryname, const bool &useConventionalData, const bool &useDTIData, const bool &usePerfData, const bool &useDistData);
  std::vector<std::map<ImageModalityType, std::string>> LoadQualifiedSubjectsFromGivenDirectoryForRecurrence(const MachineLearningApplicationSubtype type, const std::string &directoryname, const bool &useT1Data, const bool &useDTIData, const bool &usePerfData, const bool &useDistData);
  /**
  \brief Displays the next image in order based on keyboard input (1,2,3,...)
  */
  void ChangeImageWithOrder(SlicerManager *sm, int order);

  /**
  \brief Called internally within DisplayChange function to update link between images
  */
  void UpdateLinkManager(std::string id, int slicer, double x, double y, double z);

  /**
  \brief Called internally within DisplayChange function to update link between images
  */
  void UpdateLinkedNavigation( Slicer* refSlicer);

  void toolTabDockChanged(bool bUnDocked)//TBD - move to cpp file 
  {
	  if (bUnDocked)
	  {
		  
		  m_tabWidget->setMaximumHeight(m_tabWidget->minimumHeight() * 10);
		  m_toolTabdock->show();
	  }
	  else
	  {
		  m_tabWidget->setMaximumHeight(m_tabWidget->minimumHeight());
	  }
  }
  //! Returns the active tab from the tab widget
  int getActiveTabId()
  {
    return m_tabWidget->currentIndex();
  }

  //! shows if the tumor points are being initialized
  bool tumorPointSelected()
  {
    return tumorPanel->mTumorPointsSelected;
  }
  

  void ApplicationLIBRASingle();
  void ApplicationLIBRABatch();
  void ApplicationConfetti();
  void ApplicationSBRTLungField();
  void ApplicationSBRTNodule();
  void ApplicationSBRTAnalysis();

  //! Convert 2D image to 3D image with a single slice and write to temp folder
  void ConversionFrom2Dto3D(const std::string &fileName, bool loadAsImage = false);


  void ApplicationDirectionality();
#ifdef BUILD_FETALBRAIN
  void ApplicationFetalBrain();
#endif

#ifdef BUILD_EGFRvIII
  void ApplicationEGFR();
#endif
#ifdef BUILD_RECURRENCE
  void ApplicationRecurrence();
#endif
#ifdef BUILD_PSEUDOPROGRESSION
  void ApplicationPseudoProgression();
#endif
#ifdef BUILD_ATLAS
  void ApplicationPopulationAtlas();
#endif
#ifdef BUILD_ISUBTYPE
  void ApplicationImagingSubtype();
#endif
#ifdef BUILD_MSUBTYPE
  void ApplicationMolecularSubtype();
#endif
#ifdef BUILD_SURVIVAL
  void ApplicationSurvival();
#endif
#ifdef BUILD_EGFRvIIISVM
  void ApplicationEGFRvIIISVM();
#endif
#ifdef BUILD_GEODESIC
  void ApplicationGeodesic();
#endif
#ifdef BUILD_GEODESICTRAIN
  void ApplicationGeoTrain();
#endif
  void ApplicationGeodesicTreshold();
#ifdef BUILD_ITKSNAP
  void ApplicationITKSNAP();
#endif
#ifdef BUILD_WHITESTRIPE
  void ApplicationWhiteStripe();
#endif
  void ImageDenoising();
  void ImageBiasCorrection();
  void ImageRegistration();
  void ImageHistogramMatching();
  void ImageDeepMedicNormalizer();
  void ImageSkullStripping();
  void DCM2NIfTIConversion();
  void CustomPreprocessing();
  void ApplicationPCA();
  void PerfusionMeasuresCalculation();
  void DiffusionMeasuresCalculation();
  void ClassifierTraining();
  void ApplicationDeepMedicSegmentation();
  void ApplicationTheia();

  void Registration(std::string fixedfilename, std::vector<std::string> inputFileNames, std::vector<std::string> outputFileNames, std::vector<std::string> matrixFileNames, bool registrationMode, std::string metrics, bool affineMode, std::string radii, std::string iterations);

  //confirm before exit
  void closeEvent(QCloseEvent * event);

  // Progress Update
  void updateProgress(int progress, std::string message = "", int max = 100);

  void EnableAdvancedVisualizer()
  {
    m_advancedVisualizer = true;
  }  

public:

  std::string currentPlatform;

  std::string m_tempFolderLocation;
  std::string dicomfilename; // contains the first image in the DICOM series
  std::vector<SlicerManager*> mSlicerManagers;
  std::vector<SimpleImageManager*> mNonViewingImageManager;

  //QWidget* mMainWidget;
  QString mInputPathName;
  std::vector<QSlider*> verticalSliders;
  //
  std::string mCurrentSelectedImageId;
  std::string mCurrentPickedImageId;
  std::string mProjectVariant;
  int mCurrentPickedImageIndex;

  bool m_advancedVisualizer = false;

  //progress Bar
  QProgressBar *m_progressBar;
  QLabel* m_messageLabel;

  //
  Landmarks* mLandmarks;
  Landmarks* mSeedPoints;
  Landmarks* mTissuePoints;
  int mCurrentLandmarkTissueType;
  vtkSmartPointer<vtkImageData> mMask;

  SHAPE_MODE m_drawShapeMode;
  int mCurrentNearPoints;
  int mCurrentFarPoints;
  int mCurrentInitPoints;


  int mBorderStartX;
  int mBorderStartY;
  int mBorderStartZ;

  int mBorderEndX;
  int mBorderEndY;
  int mBorderEndZ;


  PreprocessingPipelineClass mPreprocessingObj;
  OutputWritingManager mOutputManager;
  NiftiDataManager mNifiDataManager;

  // Applications that need to be declared on fMainWindow level (multiple UI level interactions needed)
  RecurrenceEstimator mRecurrenceEstimator;
  PseudoProgressionEstimator mPseudoEstimator;
  SurvivalPredictor mSurvivalPredictor;
  EGFRvIIIIndexPredictor mEGFRvIIIPredictor;
  PopulationAtlases mPopulationAtlas;
  ImagingSubtypePredictor mImagingSubtype;
  MolecularSubtypePredictor mMolecularSubtype;
  Fetalbrain mfetalbrain;


  fHelpDialog* mHelpDlg;
  fHelpTutorial mHelpTutorial;

  std::string t1cePath;
  std::string m_imagetype_string;
  Ui::fMainWindow	m_mainWindowUI;

  std::vector<int> mActionIds;
  std::vector<int> mActionSequenceIds;
  
  std::vector< std::vector<PointVal> > mActionPoints;
  
  GenericImage::Pointer mCustomImageToThreshold;

  int mSequenceNumber, mCustomImageToThreshold_min, mCustomImageToThreshold_max;



  private:
    ImageTypeFloat3D::Pointer m_InputGeomasks;
    ImageTypeShort3D::Pointer m_imgGeodesicOut;
    ImageTypeShort3D::Pointer m_imgGeodesicOutPositive;
    ImageTypeShort3D::Pointer m_imgGeodesicOutNegative;
    std::map<std::string, float> m_fetalbrainfeatures;
    int m_fetalslice;

    struct DicomDictTagAndVal
    {
      std::string tag;
      std::string value;

      DicomDictTagAndVal(const std::string &input_tag, const std::string &input_value) :
        tag(input_tag), value(input_value)
      { }

      DicomDictTagAndVal(const std::string &input_tag) :
      tag(input_tag)
      { }

      void SetValue(const std::string &input_value)
      {
        value = input_value;
      }
    };

    bool m_skipTutorialOnNextRun = false;

    int startExternalProcess(const QString &application, const QStringList &arguments);
};
#if __GNUC__
#pragma GCC visibility pop
#endif
//-------------------------------------------------------------------------------------

#endif
