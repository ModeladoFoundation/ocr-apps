#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <cmath>
#include <set>
#include <list>
#include <cassert>

struct Detection
{
    double x;
    double y;
    double rho;
};

bool operator<(const Detection & d1, const Detection & d2)
{
    if (d1.x < d2.x)
    {
        return true;
    }
    else if (d1.x == d2.x)
    {
        if (d1.y  < d2.y) { return true; }
        else { return false; }
    }
    else // d1.x > d2.x
    {
        return false;
    }
}

static void PrintProgress(const std::set<Detection> & s)
{
#ifndef RAG
    if (s.size() % 100 == 0)
    {
        if (s.size() == 0)
        {
            printf("\rDetections remaining to cluster... complete.");
            fflush(stdout);
        }
        else
        {
            printf("\rDetections remaining to cluster... %lu",
                s.size()); fflush(stdout);
        }
    }
#endif
}

int main(int argc, char **argv)
{
    if (argc != 2)
    {
        fprintf(stderr, "Usage: %s <detections-input-file>\n", argv[0]);
        exit(-1);
    }

    FILE *fp = fopen(argv[1], "rb");
    if (fp == NULL)
    {
        fprintf(stderr, "Error: unable to open detection list file %s.\n",
            argv[1]);
        exit(-1);
    }

    Detection detection;
    std::set<Detection> unclustered;

    char *line = NULL;
    size_t length = 0;
    while (getline(&line, &length, fp) > 0)
    {
        if (strlen(line) == 1 && line[0] == '\n') { continue; }
        int n = sscanf(line, "(x=%lf m, y=%lf m, p=%lf)\n", &detection.x, &detection.y, &detection.rho);
        if (n != 3)
        {
            fprintf(stderr, "Error: unexpected detection format on following line:\n");
            fprintf(stderr, "\t%s\n", line);
            exit(-1);
        }

        unclustered.insert(detection);
    }

    fclose(fp);
    free(line);
    line = NULL;

    printf("Number of detections = %lu\n", unclustered.size());

    std::list< std::set<Detection> > clusters;

    printf("Detections remaining to cluster... %lu", unclustered.size()); fflush(stdout);
    const double Rcluster = 2.0;
    while (unclustered.size() > 0)
    {
        std::set<Detection>::iterator first = unclustered.begin();
        Detection u = *first;
        unclustered.erase(u);
        PrintProgress(unclustered);

        std::set<Detection> unchecked;
        unchecked.insert(u);
        std::set<Detection> newCluster;

        while (unchecked.size() > 0)
        {
            std::set<Detection>::iterator first = unchecked.begin();
            Detection a = *first;
            unchecked.erase(first);
            newCluster.insert(a);

            std::set<Detection>::iterator iter = unclustered.begin();
            while (iter != unclustered.end())
            {
                Detection u = *iter;
                double dist = sqrt((u.x-a.x)*(u.x-a.x) + (u.y-a.y)*(u.y-a.y));
                if (dist <= Rcluster)
                {
                    std::set<Detection>::iterator toErase = iter;
                    unchecked.insert(*toErase);
                    ++iter;
                    unclustered.erase(toErase);
                    PrintProgress(unclustered);
                }
                else
                {
                    ++iter;
                }
            }
        }

        clusters.push_back(newCluster);
    }

    printf("\nNumber of clusters = %lu\n", clusters.size());

    // We should have placed all detections into some cluster
    assert(unclustered.size() == 0);

    std::list< std::set<Detection> >::const_iterator iter = clusters.begin();
    while (iter != clusters.end())
    {
        const std::set<Detection> & cluster = *iter;
        const size_t nsize = cluster.size();
        std::set<Detection>::const_iterator citer = cluster.begin();
        double mean_x = 0, mean_y = 0;
        while (citer != cluster.end())
        {
            mean_x += (*citer).x;
            mean_y += (*citer).y;
            ++citer;
        }
        printf("Cluster centroid (x,y) position is (%f, %f) and mass is %lu\n",
            mean_x/nsize, mean_y/nsize, cluster.size());
        ++iter;
    }

    return 0;
}
