# KD-Tree原理

## introduction

K-Dimension Tree

Used for searching in High Dim Space.

We search in 3D point clouds, so k = 3

kd树是一个二叉树结构, 它的每一个节点记载了【特征坐标，切分轴，指向左枝的指针，指向右枝的指针】。
其中, 特征坐标是线性空间 $\mathbb{R}^n$ 中的一个点 $\left(x_1, x_2, \ldots, x_n\right)$ 。
切分轴由一个整数 $r$ 表示, 这里 $1 \leq r \leq n$, 是我们在 $n$ 维空间中沿第 $r$ 维进行一次分割。
节点的左枝和右枝分别都是 $\mathrm{kd}$ 树 , 并且满足 : 如果 $y$ 是左枝的一个特征坐标, 那么 $y_r \leq x_r$; 并且如果 $z$ 是右枝 的一个特征坐标, 那么 $z_r \geq x_r$ 。
给定一个数据样本集 $S \subseteq R^n$ 和切分轴 $r$, 以下递归算法将构建一个基于该数据集的 $\mathrm{kd}$ 树, 每一次循环制作一个 节点 :
- 如果 $|S|=1$, 记录 $S$ 中唯一的一个点为当前节点的特征数据, 并且不设左枝和右枝。 ( $|S|$ 指集合 $S$ 中元素 的数量)
- 如果 $|S|>1$ :
- 将 $S$ 内所有点按照第 $r$ 个坐标的大小进行排序 ;
- 选出该排列后的中位元素 (如果一共有偶数个元素, 则选择中位左边或右边的元素, 左或右并无影响), 作为当前节点的特征坐标, 并且记录切分轴 $r$;
- 将 $S_L$ 设为在 $S$ 中所有排列在中位元素之前的元素; $S_R$ 设为在 $S$ 中所有排列在中位元素后的元素;
- 当前节点的左枝设为以 $S_L$ 为数据集并且 $r$ 为切分轴制作出的 $\mathrm{kd}$ 树 ; 当前节点的右枝设为以 $S_R$ 为数据集 并且 $r$ 为切分轴制作出 的 $\mathrm{kd}$ 树。再设 $r \leftarrow(r+1) \bmod n$ 。 (这里, 我们想轮流沿着每一个维度进行 分割; $\bmod n$ 是因为一共有 $n$ 个维度, 在 沿着最后一个维度进行分割之后再重新回到第一个维度。)

## Usage

- Data structure
```C++
struct kdtree{
    Node-data - 数据矢量   数据集中某个数据点，是n维矢量（这里也就是k维）
    Range     - 空间矢量   该节点所代表的空间范围
    split     - 整数       垂直于分割超平面的方向轴序号
    Left      - kd树       由位于该节点分割超平面左子空间内所有数据点所构成的k-d树
    Right     - kd树       由位于该节点分割超平面右子空间内所有数据点所构成的k-d树
    parent    - kd树       父节点  
}
```
- Construct algorithm
```
Input:  无序化的点云，维度k
Output：点云对应的kd-tree
Algorithm：
1、初始化分割轴：对每个维度的数据进行方差的计算，取最大方差的维度作为分割轴，标记为r；
2、确定节点：对当前数据按分割轴维度进行检索，找到中位数数据，并将其放入到当前节点上；
3、划分双支：
    划分左支：在当前分割轴维度，所有小于中位数的值划分到左支中；
    划分右支：在当前分割轴维度，所有大于等于中位数的值划分到右支中。
4、更新分割轴：r = (r + 1) % k;
5、确定子节点：
    确定左节点：在左支的数据中进行步骤2；
    确定右节点：在右支的数据中进行步骤2；

```
Take 2D for example:
$$
{(2,3),(5,4),(9,6),(4,7),(8,1),(7,2)}
$$

- Nearest Neighbor Search

    - Similar to KNN, but much more simple than KNN
    - Search according to dimension

Single NN

Multiple NN

## Programming

KD-Tree Nearest Neighbor Search with pcl:
```C++
//头文件
#include <pcl/kdtree/kdtree_flann.h>
//设定kd-tree的智能指针
pcl::KdTreeFLANN<pcl::PointXYZI>::Ptr kdtreeCornerLast(new pcl::KdTreeFLANN<pcl::PointXYZI>());
//输入三维点云，构建kd-tree
kdtreeCornerLast->setInputCloud(laserCloudCornerLast);
//在点云中寻找点searchPoint的k近邻的值，返回下标pointSearchInd和距离pointSearchSqDis
kdtreeCornerLast->nearestKSearch (searchPoint, K, pointIdxNKNSearch, pointNKNSquaredDistance);

```

Search by distance and range:
```C++
//在点云中寻找和点searchPoint满足radius距离的点和距离，返回下标pointIdxRadiusSearch和距离pointRadiusSquaredDistance
kdtreeCornerLast->radiusSearch (searchPoint, radius, pointIdxRadiusSearch, pointRadiusSquaredDistance)

```

Full code in PCL documentation
```C++

#include <pcl/point_cloud.h>
#include <pcl/kdtree/kdtree_flann.h>

#include <iostream>
#include <vector>
#include <ctime>

int
main (int argc, char** argv)
{
  srand (time (NULL));

  pcl::PointCloud<pcl::PointXYZ>::Ptr cloud (new pcl::PointCloud<pcl::PointXYZ>);

  // Generate pointcloud data
  cloud->width = 1000;
  cloud->height = 1;
  cloud->points.resize (cloud->width * cloud->height);

  for (std::size_t i = 0; i < cloud->points.size (); ++i)
  {
    cloud->points[i].x = 1024.0f * rand () / (RAND_MAX + 1.0f);
    cloud->points[i].y = 1024.0f * rand () / (RAND_MAX + 1.0f);
    cloud->points[i].z = 1024.0f * rand () / (RAND_MAX + 1.0f);
  }

  pcl::KdTreeFLANN<pcl::PointXYZ> kdtree;

  kdtree.setInputCloud (cloud);

  pcl::PointXYZ searchPoint;

  searchPoint.x = 1024.0f * rand () / (RAND_MAX + 1.0f);
  searchPoint.y = 1024.0f * rand () / (RAND_MAX + 1.0f);
  searchPoint.z = 1024.0f * rand () / (RAND_MAX + 1.0f);

  // K nearest neighbor search

  int K = 10;

  std::vector<int> pointIdxNKNSearch(K);
  std::vector<float> pointNKNSquaredDistance(K);

  std::cout << "K nearest neighbor search at (" << searchPoint.x 
            << " " << searchPoint.y 
            << " " << searchPoint.z
            << ") with K=" << K << std::endl;

  if ( kdtree.nearestKSearch (searchPoint, K, pointIdxNKNSearch, pointNKNSquaredDistance) > 0 )
  {
    for (std::size_t i = 0; i < pointIdxNKNSearch.size (); ++i)
      std::cout << "    "  <<   cloud->points[ pointIdxNKNSearch[i] ].x 
                << " " << cloud->points[ pointIdxNKNSearch[i] ].y 
                << " " << cloud->points[ pointIdxNKNSearch[i] ].z 
                << " (squared distance: " << pointNKNSquaredDistance[i] << ")" << std::endl;
  }

  // Neighbors within radius search

  std::vector<int> pointIdxRadiusSearch;
  std::vector<float> pointRadiusSquaredDistance;

  float radius = 256.0f * rand () / (RAND_MAX + 1.0f);

  std::cout << "Neighbors within radius search at (" << searchPoint.x 
            << " " << searchPoint.y 
            << " " << searchPoint.z
            << ") with radius=" << radius << std::endl;


  if ( kdtree.radiusSearch (searchPoint, radius, pointIdxRadiusSearch, pointRadiusSquaredDistance) > 0 )
  {
    for (std::size_t i = 0; i < pointIdxRadiusSearch.size (); ++i)
      std::cout << "    "  <<   cloud->points[ pointIdxRadiusSearch[i] ].x 
                << " " << cloud->points[ pointIdxRadiusSearch[i] ].y 
                << " " << cloud->points[ pointIdxRadiusSearch[i] ].z 
                << " (squared distance: " << pointRadiusSquaredDistance[i] << ")" << std::endl;
  }


  return 0;
}
```

## Reference
https://www.zhihu.com/collection/719959990
https://www.joinquant.com/view/community/detail/c2c41c79657cebf8cd871b44ce4f5d97
https://en.wikipedia.org/wiki/K-d_tree
