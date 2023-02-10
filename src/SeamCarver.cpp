#include "SeamCarver.h"

#include <algorithm>
#include <cmath>
#include <vector>

SeamCarver::Seam SeamCarver::FindSeam(const bool is_vertical) const
{

    std::vector<std::vector<double>> distance(width, std::vector<double>(height, 0));

    size_t main_size = height;
    size_t sub_size = width;
    size_t main_coord;
    size_t sub_coord = 0;

    size_t * x = &sub_coord;
    size_t * y = &main_coord;

    if (is_vertical) {
        x = &main_coord;
        y = &sub_coord;
        main_size = width;
        sub_size = height;
    }

    for (main_coord = 0; main_coord < main_size; main_coord++) {
        distance[*x][*y] = GetPixelEnergy(*x, *y);
    }

    for (sub_coord = 1; sub_coord < sub_size; sub_coord++) {
        for (main_coord = 0; main_coord < main_size; main_coord++) {
            double min_dist;
            sub_coord--;
            double part1 = distance[*x][*y]; // main   sub-
            if (main_coord == 0) {
                main_coord++;
                min_dist = std::min(part1, distance[*x][*y]); // main+   sub-
                main_coord--;
            }
            else if (main_coord == main_size - 1) {
                main_coord--;
                min_dist = std::min(part1, distance[*x][*y]); // main-   sub-
                main_coord++;
            }
            else {
                main_coord--;
                double part2 = distance[*x][*y]; // main-   sub-
                main_coord += 2;
                min_dist = std::min(std::min(part2, part1), distance[*x][*y]); // main+   sub-
                main_coord--;
            }
            sub_coord++;
            distance[*x][*y] = GetPixelEnergy(*x, *y) + min_dist;
        }
    }

    Seam result;
    main_coord = 0;
    sub_coord = sub_size - 1;
    double cur_min = distance[*x][*y];
    size_t cur_min_index = 0;

    for (main_coord = 1; main_coord < main_size; main_coord++) {
        if (distance[*x][*y] < cur_min) {
            cur_min = distance[*x][*y];
            cur_min_index = main_coord;
        }
    }
    result.push_back(cur_min_index);

    main_coord = cur_min_index;

    for (sub_coord = sub_size - 1; sub_coord > 0; sub_coord--) {
        sub_coord--;
        double part1 = distance[*x][*y]; // main   sub-
        if (main_coord == 0) {
            main_coord++;
            if (part1 <= distance[*x][*y]) { // main+   sub-
                result.push_back(main_coord - 1);
            }
            else {
                result.push_back(main_coord);
            }
            main_coord--;
        }
        else if (main_coord == main_size - 1) {
            main_coord--;
            if (part1 <= distance[*x][*y]) { // main-   sub-
                result.push_back(main_coord + 1);
            }
            else {
                result.push_back(main_coord);
            }
            main_coord++;
        }
        else {
            main_coord--;
            double part2 = distance[*x][*y]; // main-   sub-
            main_coord += 2;
            if (part1 < part2) {
                if (part1 > distance[*x][*y]) { // main+   sub-
                    result.push_back(main_coord);
                }
                else {
                    result.push_back(main_coord - 1);
                }
            }
            else {
                if (part2 > distance[*x][*y]) { // main+   sub-
                    result.push_back(main_coord);
                }
                else {
                    result.push_back(main_coord - 2);
                }
            }
            main_coord--;
        }
        sub_coord++;

        main_coord = result.back();
    }

    std::reverse(result.begin(), result.end());
    return result;
}

SeamCarver::SeamCarver(Image image)
    : m_image(std::move(image))
    , width(m_image.m_table.size())
    , height(m_image.m_table[0].size())
{
}

const Image & SeamCarver::GetImage() const
{
    return m_image;
}

size_t SeamCarver::GetImageWidth() const
{
    return width;
}

size_t SeamCarver::GetImageHeight() const
{
    return height;
}

double SeamCarver::GetPixelEnergy(size_t columnId, size_t rowId) const
{
    Image::Pixel left = m_image.GetPixel((width + columnId - 1) % width, rowId);
    Image::Pixel right = m_image.GetPixel((width + columnId + 1) % width, rowId);
    Image::Pixel up = m_image.GetPixel(columnId, (height + rowId - 1) % height);
    Image::Pixel down = m_image.GetPixel(columnId, (height + rowId + 1) % height);

    double rx = left.m_red - right.m_red;
    double gx = left.m_green - right.m_green;
    double bx = left.m_blue - right.m_blue;
    double ry = up.m_red - down.m_red;
    double gy = up.m_green - down.m_green;
    double by = up.m_blue - down.m_blue;

    double t = (rx * rx) + (gx * gx) + (bx * bx) + (ry * ry) + (gy * gy) + (by * by);

    return std::sqrt(t);
}

SeamCarver::Seam SeamCarver::FindHorizontalSeam() const
{
    if (height == 1) {
        return Seam(width, 0);
    }

    return FindSeam(false);
}

SeamCarver::Seam SeamCarver::FindVerticalSeam() const
{
    if (width == 1) {
        return Seam(height, 0);
    }

    return FindSeam(true);
}

void SeamCarver::RemoveHorizontalSeam(const Seam & seam)
{
    for (size_t x = 0; x < width; x++) {
        m_image.m_table[x].erase(m_image.m_table[x].begin() + seam[x]);
    }
    height--;
}

void SeamCarver::RemoveVerticalSeam(const Seam & seam)
{
    for (size_t y = 0; y < height; y++) {
        for (size_t x = seam[y] + 1; x < width; x++) {
            m_image.m_table[x - 1][y] = m_image.m_table[x][y];
        }
    }
    m_image.m_table.pop_back();
    width--;
}
