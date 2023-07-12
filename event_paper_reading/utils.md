# Utils of Event Paper Reading

## Table of Contents

## Categories

### 1.Event based vision

#### 1.1 Survey

- Event-based vision: A survey

- Event-based Simultaneous Localization and Mapping: A Comprehensive Survey
    - Code: https://github.com/kun150kun/ESLAM-survey

- Deep Learning for Event-based Vision: A Comprehensive Survey and Benchmarks
    - Code: https://github.com/vlislab2022/Awesome-Events-Deep-Learning


#### 1.2 Dataset and benchmark

- VECtor: A Versatile Event-Centric Benchmark for Multi-Sensor SLAM
    - Dataset: https://star-datasets.github.io/vector/

- ViViD++ : Vision for Visibility Dataset
    - Code: https://visibilitydataset.github.io/

- M3ED: Multi-Robot, Multi-Sensor, Multi-Environment Event Dataset
    - Code: https://github.com/daniilidis-group/m3ed.
#### 1.3 Feature

- eCDT: Event Clustering for Simultaneous Feature Detection and Tracking

- Event Camera Point Cloud Feature Analysis and Shadow Removal for Road Traffic Sensing

- Learning Local Event-based Descriptor for Patch-based Stereo Matching

- Neuromorphic Event-Based Generalized Time-Based Stereovision

- EventPoint: Self-Supervised Interest Point Detection and Description for Event-based Camera

- [CVPR2023] Event-based Shape from Polarization
    - Code: https://rpg.ifi.uzh.ch/esfp.html

- [CVPR2023] Data-driven Feature Tracking for Event Cameras
    - Code: https://github.com/uzh-rpg/deep_ev_tracker

- [CVPR2023] Adaptive Global Decay Process for Event Cameras
    - Code: https://github.com/neuromorphicparis/event_batch


#### 1.4 Fusion

- Fusing Event-based and RGB camera for Robust Object Detection in Adverse Conditions

- Frame-Event Alignment and Fusion Network for High Frame Rate Tracking

- Event-Based Fusion for Motion Deblurring with Cross-modal Attention

- [CVPR2023] Frame-Event Alignment and Fusion Network for High Frame Rate Tracking

- Event-IMU fusion strategies for faster-than-IMU estimation throughput


#### 1.5 Motion Estimation

- Robust Motion Compensation for Event Cameras With Smooth Constraint

- Globally-Optimal Event Camera Motion Estimation

- [CVPR2023] Progressive Spatio-temporal Alignment for Efficient Event-based Motion Estimation
    - Code: https://github.com/huangxueyan/PEME
#### 1.6 Point Cloud

- Detecting Space Objects in Event Camera Data through 3D Point Cloud Processing

- Efficient Human Pose Estimation via 3D Event Point Cloud
    - Code: https://github.com/MasterHow/EventPointPose

- 3D Object Detection with Pointformer

#### 1.7 3D Reconstruction

- Event-Based Dense Reconstruction Pipeline

- [CVPR2023] EventNeRF: Neural Radiance Fields from a Single Colour Event Camera
    - Code: https://4dqv.mpi-inf.mpg.de/EventNeRF

#### 1.8 SLAM

- Event-Based Line SLAM in Real-Time


#### 1.9 SNN

- Performance comparison of DVS data spatial downscaling methods using Spiking Neural Networks
    - Code: https: //github.com/amygruel/EvVisu


#### 1.10 Odometry
-  Event Camera-based Visual Odometry for Dynamic Motion Tracking of a Legged Robot Using Adaptive Time Surface
    - Adaptive Time Surface (ATS)
    - pixel selection and filtering
    - simultaneous optimization over RGB and event data

- ESVIO: Event-based Stereo Visual Inertial Odometry
    - Code: https://github.com/arclab-hku/Event_based_VO-VIO-SLAM

- DEVO: Depth-Event Camera Visual Odometry in Challenging Conditions

#### 1.11 Optical Flow Estimation
- [CVPR2023] Tangentially Elongated Gaussian Belief Propagation for Event-based Incremental Optical Flow Estimation
    - Code: https://github.com/DensoITLab/tegbp/

#### 1.12 Object detection
- [CVPR2023] Recurrent Vision Transformers for Object Detection with Event Cameras
    - Code: https://github.com/uzh-rpg/RVT

#### 1.13 Video interpolation and Super-Resolution
- [CVPR2023] Video Event Restoration Based on Keyframes for Video Anomaly Detection

- [CVPR2023] Learning Spatial-Temporal Implicit Neural Representations for Event-Guided Video Super-Resolution
    - Code: https://vlis2022.github.io/cvpr23/egvsr
- [CVPR2023] Learning Event Guided High Dynamic Range Video Reconstruction
    - Code: https://yixinyang-00.github.io/HDRev/

- [CVPR2023] Event-based Video Frame Interpolation with Cross-Modal Asymmetric Bidirectional Motion Fields
    - https: //github.com/intelpro/CBMNet

- [CVPR2023] Event-Based Frame Interpolation with Ad-hoc Deblurring
    - https://github.com/AHupuJR/REFID

- [CVPR2023] Event-based Blurry Frame Interpolation under Blind Exposure
    - https://github.com/WarranWeng/EBFI-BE

#### 1.14 Depth sensing
- [CVPR2023] Hierarchical Neural Memory Network for Low Latency Event Processing
    - Code: https://hamarh.github.io/hmnet/

- [CVPR2023] Learning Adaptive Dense Event Stereo from the Image Domain

- [CVPR2023] High-fidelity Event-Radiance Recovery via Transient Event Frequency
    - Code: https://github.com/hjynwa/TEF

#### 1.15 Object tracking

- [CVPR2023] Event-guided Person Re-Identification via Sparse-Dense Complementary Learning
    - Code: https://github.com/ChengzhiCao/SDCL

#### 1.16 Denoise/Deblur

#### 1.17 Segmentation
- Asynchronous Events-based Panoptic Segmentation using Graph Mixer Neural Network
    - Code: https://github.com/sanket0707/GNN-Mixer

- [CVPR2023] All-in-focus Imaging from Event Focal Stack
    - https://hylz-2019.github.io/EFS

#### 1.18 Servoing

- Neuromorphic Eye-in-Hand Visual Servoing
    - **Paper:** Neuromorphic Eye-in-Hand Visual Servoing
### 2.3D Object Detectioon

- PIXOR: Real-time 3D Object Detection from Point Clouds:

- enter-based 3D Object Detection and Tracking

- 3D Object Detection with Pointformer


### ICRA 2023 Papers

|Title|Session|Abstract|
|----|----|----|
|Event-Based Real-Time Moving Object Detection Based on IMU Ego-Motion Compensation|Sensor Fusion I|Accurate and timely onboard perception is a prerequisite for mobile robots to operate in highly dynamic scenarios. The bio-inspired event camera can capture more motion details than a traditional camera by triggering each pixel asynchronously and therefore is more suitable in such scenarios. Among various perception tasks based on the event camera, ego-motion removal is one fundamental procedure to reduce perception ambiguities. Recent ego-motion removal methods are mainly based on optimization processes and may be computationally expensive for robot applications. In this paper, we consider the challenging perception task of detecting fast-moving objects from an aggressively operated platform equipped with an event camera, achieving computational cost reduction by directly employing IMU motion measurement. First, we design a nonlinear warping function to capture rotation information from an IMU and to compensate for the camera motion during an asynchronous events stream. The proposed nonlinear warping accuracy by 10%-15%. Afterward, we segmented the moving parts on the warped image through dynamic threshold segmentation and optical flow calculation, and clustering. Finally, we validate the proposed detection pipeline on public datasets and real-world data streams containing challenging light conditions and fast-moving objects.|
|Fast Event-Based Double Integral for Real-Time Robotics|Visual Tracking|Motion deblurring is a critical ill-posed problem that is important in many vision-based robotics applications. The recently proposed event-based double integral (EDI) provides a theoretical framework for solving the deblurring problem with the event camera and generating clear images at high frame-rate. However, the original EDI is mainly designed for offline computation and does not support real-time requirement in many robotics applications. In this paper, we propose the fast EDI, an efficient implementation of EDI that can achieve real-time online computation on single-core CPU devices, which is common for physical robotic platforms used in practice. In experiments, our method can handle event rates at as high as 13 million event per second in a wide variety of challenging lighting conditions. We demonstrate the benefit on multiple downstream real-time applications, including localization, visual tag detection, and feature matching.|
|Continuous-Time Gaussian Process Motion-Compensation for Event-Vision Pattern Tracking with Distance Fields|Visual Tracking|This work addresses the issue of motion compensation and pattern tracking in event camera data. An event camera generates asynchronous streams of events triggered independently by each of the pixels upon changes in the observed intensity. Providing great advantages in low-light and rapid-motion scenarios, such unconventional data present significant research challenges as traditional vision algorithms are not directly applicable to this sensing modality. The proposed method decomposes the tracking problem into a local SE(2) motion-compensation step followed by a homography registration of small motion-compensated event batches. The first component relies on Gaussian Process (GP) theory to model the continuous occupancy field of the events in the image plane and embed the camera trajectory in the covariance kernel function. In doing so, estimating the trajectory is done similarly to GP hyperparameter learning by maximising the log marginal likelihood of the data. The continuous occupancy fields are turned into distance fields and used as templates for homography-based registration. By benchmarking the proposed method against other state-of-the-art techniques, we show that our open-source implementation performs high-accuracy motion compensation and produces high-quality tracks in real-world scenarios.|
|Fusion of Events and Frames Using 8-DOF Warping Model for Robust Feature Tracking|Visual Tracking|Event cameras are asynchronous neuromorphic vision sensors with high temporal resolution and no motion blur, offering advantages over standard frame-based cameras especially in high-speed motions and high dynamic range conditions. However, event cameras are unable to capture the overall context of the scene, and produce different events for the same scenery depending on the direction of the motion, creating a challenge in data association. Standard camera, on the other hand, provides frames at a fixed rate that are independent of the motion direction, and are rich in context. In this paper, we present a robust feature tracking method that employs 8-DOF warping model in minimizing the difference between brightness increment patches from events and frames, exploiting the complementary nature of the two data types. Unlike previous works, the proposed method enables tracking of features under complex motions accompanying distortions. Extensive quantitative evaluation over publicly available datasets was performed where our method shows an improvement over state-of-the-art methods in robustness with greatly prolonged feature age and in accuracy for challenging scenarios.|
|Direct Angular Rate Estimation without Event Motion-Compensation at High Angular Rates|Vision-Based Navigation I|Feature-based methods are a popular method for camera state estimation using event cameras. Due to the spatiotemporal nature of events, all event images exhibit smearing of events analogous to motion blur for a camera under motion. As such, events must be motion compensated to derive a sharp event image. However, this presents a causality dilemma where motion prior is required to unsmear the events, but a sharp event image is required to estimate motion. While it is possible to use the IMU to develop motion prior, it has been shown that the limited dynamic range of ±2000 ◦/s is insufficient for high angular rate rotorcrafts. Furthermore, smoothing of motion-compensated images due to actual event detection time latency in event cameras severely limits the performance of feature-based methods at high angular rates. This paper proposes a Fourier-based angular rate estimator capable of estimating angular rates directly on non-motion compensated event images. This method circumvents the need for external motion priors in camera state estimation and sidesteps problematic smoothing of features in the spatial domain due to motion blur. Lastly, using an NVIDIA Jetson Xavier NX, the algorithm is demonstrated to be real-time performant up to 3960◦/s|
|Fusing Event-Based Camera and Radar for SLAM Using Spiking Neural Networks with Continual STDP Learning|Sensor Fusion II|This work proposes a first-of-its-kind SLAM architecture fusing an event-based camera and a Frequency Modulated Continuous Wave (FMCW) radar for drone navigation. Each sensor is processed by a bio-inspired Spiking Neural Network (SNN) with continual Spike-Timing-Dependent Plasticity (STDP) learning, as observed in the brain. In contrast to most learning-based SLAM systems, our method does not require an offline training phase but rather, the SNN continuously learns features from the input data on the fly via STDP. At the same time, the SNN outputs are used as feature descriptors for loop closure detection and map correction. We conduct numerous experiments to benchmark our system against state-of-the-art RGB methods and we demonstrate the robustness of our DVS-Radar SLAM approach against strong lighting variations.|
|Event-Triggered Optimal Formation Tracking Control Using Reinforcement Learning for Large-Scale UAV Systems|Optimization and Optimal Control|Large-scale UAV switching formation tracking control has been widely applied in many fields such as search and rescue, cooperative transportation, and UAV light shows. In order to optimize the control performance and reduce the computational burden of the system, this study proposes an event-triggered optimal formation tracking controller for discrete-time large-scale UAV systems (UASs). And an optimal decision - optimal control framework is completed by introducing the Hungarian algorithm and actor-critic neural networks (NNs) implementation. Finally, a large-scale mixed reality experimental platform is built to verify the effectiveness of the proposed algorithm, which includes large-scale virtual UAV nodes and limited physical UAV nodes. This compensates for the limitations of the experimental field and equipment in real-world scenario, ensures the experimental safety, significantly reduces the experimental cost, and is suitable for realizing large-scale UAV formation light shows.|
|Aggregation Functions for Simultaneous Attitude and Image Estimation with Event Cameras at High Angular Rates|Aerial Robots and Autonomous Agents|For fast-moving event cameras, projection of events onto the image frame exhibits smearing of events analogous to high motion blur. For camera attitude estimation, this presents a causality dilemma where motion prior is required to unsmear events, but an image prior is required to estimate motion. This dilemma is typically circumvented by including an IMU to provide motion priors. However, IMUs limited dynamic range of ±2000 ◦/s are shown to be insufficient for high angular rate rotorcrafts. Contrast Maximization is an event-only optimization framework that computes the optimal motion compensation parameter while generating an event image simultaneously. This paper analyses the performance of existing aggregation functions of the contrast maximization framework and proposes a non-convolution-based aggregation function that outperforms existing implementations. The use of discrete event images for optimizers is discussed, demonstrating alternate avenues of the framework to exploit. The effect of motion blur in motion-compensated images is defined and studied for Contrast Maximisation at high angular rates. Lastly, the framework is applied to rotation datasets with angular rates exceeding 2000 ◦/s to demonstrate high angular rate motion estimation without motion priors.|
|DOTIE - Detecting Objects through Temporal Isolation of Events Using a Spiking Architecture|Object Detection I|Vision-based autonomous navigation systems rely on fast and accurate object detection algorithms to avoid obstacles. Algorithms and sensors designed for such systems need to be computationally efficient, due to the limited energy of the hardware used for deployment. Biologically inspired event cameras are a good candidate as a vision sensor for such systems due to their speed, energy efficiency, and robustness to varying lighting conditions. However, traditional computer vision algorithms fail to work on event-based outputs, as they lack photometric features such as light intensity and texture. In this work, we propose a novel technique that utilizes the temporal information inherently present in the events to efficiently detect moving objects. Our technique consists of a lightweight spiking neural architecture that is able to separate events based on the speed of the corresponding objects. These separated events are then further grouped spatially to determine object boundaries. This method of object detection is both asynchronous and robust to camera noise. In addition, it shows good performance in scenarios with events generated by static objects in the background, where existing event-based algorithms fail. We show that by utilizing our architecture, autonomous navigation systems can have minimal latency and energy overheads for performing object detection.|
|Improved Event-Based Dense Depth Estimation Via Optical Flow Compensation|Depth Estimation and RGB-D Sensing|Event cameras have the potential to overcome the limitations of classical computer vision in real-world applications. Depth estimation is a crucial step for high-level robotics tasks and has attracted much attention from the community. In this paper, we propose an event-based dense depth estimation architecture, Mixed-EF2DNet, which firstly predicts inter-grid optical flow to compensate for lost temporal information, and then estimates multiple contextual depth maps that are fused to generate a robust depth estimation map. To supervise the network training, we further design a smoothing loss function used to smooth local depth estimates and facilitate estimating reasonable depth for pixels without events. In addition, we introduce SE-resblocks in the depth network to enhance the network representation by selecting feature channels. Experimental evaluations on both real-world and synthetic datasets show that our method performs better in terms of accuracy when compared to state-of-the-art algorithms, especially in scene detail estimation. Besides, our method demonstrates excellent generalization in cross-dataset tasks.|
|Adaptive-SpikeNet: Event-Based Optical Flow Estimation Using Spiking Neural Networks with Learnable Neuronal Dynamics|Machine Learning for Perception|Event-based cameras have recently shown great potential for high-speed motion estimation owing to their ability to capture temporally rich information asynchronously. Spiking Neural Networks (SNNs), with their neuro-inspired event-driven processing can efficiently handle such asynchronous data, while neuron models such as the leaky-integrate and fire (LIF) can keep track of the quintessential timing information contained in the inputs. SNNs achieve this by maintaining a dynamic state in the neuron memory, retaining important information while forgetting redundant data over time. Thus, we posit that SNNs would allow for better performance on sequential regression tasks compared to similarly sized Analog Neural Networks (ANNs). However, deep SNNs are difficult to train due to vanishing spikes at later layers. To that effect, we propose an adaptive fully-spiking framework with learnable neuronal dynamics to alleviate the spike vanishing problem. We utilize surrogate gradient-based backpropagation through time (BPTT) to train our deep SNNs from scratch. We validate our approach for the task of optical flow estimation on the Multi-Vehicle Stereo Event-Camera (MVSEC) dataset and the DSEC-Flow dataset. Our experiments on these datasets show an average reduction of ~13% in average endpoint error (AEE) compared to state-of-the-art ANNs. We also explore several down-scaled models and observe that our SNN models consistently outperform similarly sized ANNs offering ~10%-16% lower AEE. These results demonstrate the importance of SNNs for smaller models and their suitability at the edge. In terms of efficiency, our SNNs offer substantial savings in network parameters ~48x and computational energy ~10.2x while attaining 10% lower EPE compared to the state-of-the-art ANN implementations.|
|How Many Events Do You Need? Event-Based Visual Place Recognition Using Sparse but Varying Pixels|Localisation 2|Event cameras continue to attract interest due to desirable characteristics such as high dynamic range, low latency, virtually no motion blur, and high energy efficiency. A potential application that would benefit from these characteristics lies in visual place recognition for robot localization, i.e. matching a query observation to the corresponding reference place in the database. This paper explores the distinctiveness of event streams from a small subset of pixels (in the tens or hundreds). We demonstrate that the absolute difference in the number of events at those pixel locations accumulated into event frames can be sufficient for place recognition when pixels that display large variations in the reference set are used. Using such sparse (over image coordinates) but varying (variance over the number of events per pixel location) pixels enables frequent and computationally cheap updates of the location estimates. Furthermore, when event frames contain a constant number of events, our method takes full advantage of the event-driven nature of the sensory stream and is robust to changes in velocity. We evaluate our proposed approach on the Brisbane-Event-VPR dataset in an outdoor driving scenario, as well as the newly contributed indoor QCR-Event-VPR dataset that was captured with a DAVIS346 camera mounted on a mobile robotic platform. Our results show that our approach achieves competitive performance when compared to several baseline methods on those datasets.|
|Real-Time Hetero-Stereo Matching for Event and Frame Camera with Aligned Events Using Maximum Shift Distance|Computer Vision and Visual Servoing|Event cameras can show better performance than frame cameras in challenging scenarios such as fast-moving environments or high-dynamic-range scenes. However, it is still difficult for event cameras to replace frame cameras in non-challenging normal scenarios. In order to leverage the advantages of both cameras, we conduct a study for the heterogeneous stereo camera system which employs both an event and a frame camera. The proposed system estimates the semi-dense disparity in real-time by matching heterogeneous data of an event and a frame camera in stereo. We propose an accurate, intuitive and efficient way to align events with 6-DOF camera motion, by suggesting the maximum shift distance method. The aligned event image shows high similarity to the edge image of the frame camera. The proposed method can estimate poses of an event camera and depth of events in a few frames, which can speed up the initialization of the event camera system. We verified our algorithm in the DSEC dataset. The proposed heterostereo matching outperformed other methods. For real-time operation, we implemented our code using parallel computation with CUDA and release our code open source: https://github.com/Haram-kim/Hetero Stereo Matching|
|RGB-Event Fusion for Moving Object Detection in Autonomous Driving|Self-Driving Cars II|Moving Object Detection (MOD) is a critical vision task for successfully achieving safe autonomous driving. Despite plausible results of deep learning methods, most existing approaches are only frame-based and may fail to reach reasonable performance when dealing with dynamic traffic participants. Recent advances in sensor technologies, especially the Event camera, can naturally complement the conventional camera approach to better model moving objects. However, event-based works often adopt a pre-defined time window for event representation, and simply integrate it to estimate image intensities from events, neglecting much of the rich temporal information from the available asynchronous events. Therefore, from a new perspective, we propose RENet, a novel RGB-Event fusion Network, that jointly exploits the two complementary modalities to achieve more robust MOD under challenging scenarios for autonomous driving. Specifically, we first design a temporal multi-scale aggregation module to fully leverage event frames from both the RGB exposure time and larger intervals. Then we introduce a bi-directional fusion module to attentively calibrate and fuse multi-modal features. To evaluate the performance of our network, we carefully select and annotate a sub-MOD dataset from the commonly used DSEC dataset. Extensive experiments demonstrate that our proposed method performs significantly better than the state-of-the-art RGB-Event fusion alternatives. The source code and dataset are publicly available at: https://github.com/ZZY-Zhou/RENet.|
|E-VFIA : Event-Based Video Frame Interpolation with Attention|Deep Learning for Visual Perception II|Video frame interpolation (VFI) is a fundamental vision task that aims to synthesize several frames between two consecutive original video images. Most algorithms aim to accomplish VFI by using only keyframes, which is an ill-posed problem since the keyframes usually do not yield any accurate precision about the trajectories of the objects in the scene. On the other hand, event-based cameras provide more precise information between the keyframes of a video. Some recent state-of-the-art event-based methods approach this problem by utilizing event data for better optical flow estimation to interpolate for video frame by warping. Nonetheless, those methods heavily suffer from the ghosting effect. On the other hand, some of kernel-based VFI methods that only use frames as input, have shown that deformable convolutions, when backed up with transformers, can be a reliable way of dealing with long-range dependencies. We propose event-based video frame interpolation with attention (E-VFIA), as a lightweight kernel-based method. E-VFIA fuses event information with standard video frames by deformable convolutions to generate high quality interpolated frames. The proposed method represents events with high temporal resolution and uses a multi-head self-attention mechanism to better encode event-based information, while being less vulnerable to blurring and ghosting artifacts; thus, generating crispier frames. The simulation results show that the proposed technique outperforms current state-of-the-art methods (both frame and event-based) with a significantly smaller model size.|
|L2E: Lasers to Events for 6-DoF Extrinsic Calibration of Lidars and Event Cameras|Calibration and Identification|As neuromorphic technology is maturing, its application to robotics and autonomous vehicle systems has become an area of active research. In particular, event cameras have emerged as a compelling alternative to frame-based cameras in low-power and latency-demanding applications. To enable event cameras to operate alongside staple sensors like lidar in perception tasks, we propose a direct, temporally-decoupled extrinsic calibration method between event cameras and lidars. The high dynamic range, high temporal resolution, and low-latency operation of event cameras are exploited to directly register lidar laser returns, allowing information-based correlation methods to optimize for the 6-DoF extrinsic calibration between the two sensors. This paper presents the first direct calibration method between event cameras and lidars, removing dependencies on frame-based camera intermediaries and/or highly-accurate hand measurements. Code: https://github.com/kev-in-ta/l2e|
|Real-Time Event Simulation with Frame-Based Cameras|Simulation and Sim2Real|Event cameras are becoming increasingly popular in robotics and computer vision due to their beneficial properties, e.g., high temporal resolution, high bandwidth, almost no motion blur, and low power consumption. However, these cameras remain expensive and scarce in the market, making them inaccessible to the majority. Using event simulators minimizes the need for real event cameras to develop novel algorithms. However, due to the computational complexity of the simulation, the event streams of existing simulators cannot be generated in real-time but rather have to be pre-calculated from existing video sequences or pre-rendered and then simulated from a virtual 3D scene. Although these offline generated event streams can be used as training data for learning tasks, all response time dependent applications cannot benefit from these simulators yet, as they still require an actual event camera. This work proposes simulation methods that improve the performance of event simulation by two orders of magnitude (making them real-time capable) while remaining competitive in the quality assessment.|
|Towards Bridging the Space Domain Gap for Satellite Pose Estimation Using Event Sensing|Space Robotics|Deep models trained using synthetic data require domain adaptation to bridge the gap between the simulation and target environments. State-of-the-art domain adaptation methods often demand sufficient amounts of (unlabelled) data from the target domain. However, this need is difficult to fulfil when the target domain is an extreme environment, such as space. In this paper, our target problem is close proximity satellite pose estimation, where it is costly to obtain images of satellites from actual rendezvous missions. We demonstrate that event sensing offers a promising solution to generalise from the simulation to the target domain under stark illumination differences. Our main contribution is an event-based satellite pose estimation technique, trained purely on synthetic event data with basic data augmentation to improve robustness against practical (noisy) event sensors. Underpinning our method is a novel dataset with carefully calibrated ground truth, comprising of real event data obtained by emulating satellite rendezvous scenarios in the lab under drastic lighting conditions. Results on the dataset showed that our event-based satellite pose estimation method, trained only on synthetic data without adaptation, could generalise to the target domain effectively.|
|Event-Based Agile Object Catching with a Quadrupedal Robot|Legged Robots|Quadrupedal robots are conquering search-and-rescue applications due to their capability to navigate challenging uneven terrains. Exteroceptive information greatly enhances this capability since perceiving their surroundings allows them to adapt their controller and thus achieve higher levels of robustness. However, sensors such as LiDARs and RGB cameras do not provide sufficient information to quickly and precisely react in a highly dynamic environment since they suffer from a bandwidth-latency tradeoff. They require significant bandwidth at high frame rates while featuring significant perceptual latency at lower frame rates, thereby limiting their versatility on resource constrained platforms. In this work, we tackle this problem by equipping our quadruped with an event camera, which does not suffer from this tradeoff due to its asynchronous and sparse operation. In levering the low latency of the events, we push the limits of quadruped agility and demonstrating high-speed ball catching with a net for the first time. We show that our our quadruped equipped with an event-camera can catch objects at maximum speeds of 15 m/s from 4 meters, with a success rate of 83%. With a VGA event camera, our method runs at 100 Hz on an NVIDIA Jetson Orin.|


