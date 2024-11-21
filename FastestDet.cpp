#include <math.h>
#include <algorithm>

#include "net.h"
#include "benchmark.h"
#include <opencv2/opencv.hpp>

// const char *class_names[] = 
// {
//     "person", "bicycle", "car", "motorcycle", "airplane", "bus", "train", "truck", "boat", "traffic light",
//     "fire hydrant", "stop sign", "parking meter", "bench", "bird", "cat", "dog", "horse", "sheep", "cow",
//     "elephant", "bear", "zebra", "giraffe", "backpack", "umbrella", "handbag", "tie", "suitcase", "frisbee",
//     "skis", "snowboard", "sports ball", "kite", "baseball bat", "baseball glove", "skateboard", "surfboard",
//     "tennis racket", "bottle", "wine glass", "cup", "fork", "knife", "spoon", "bowl", "banana", "apple",
//     "sandwich", "orange", "broccoli", "carrot", "hot dog", "pizza", "donut", "cake", "chair", "couch",
//     "potted plant", "bed", "dining table", "toilet", "tv", "laptop", "mouse", "remote", "keyboard", "cell phone",
//     "microwave", "oven", "toaster", "sink", "refrigerator", "book", "clock", "vase", "scissors", "teddy bear",
//     "hair drier", "toothbrush"
// };

const char *class_names[] = 
{
    "person"
};

float Sigmoid(float x)
{
    return 1.0f / (1.0f + exp(-x));
}

float Tanh(float x)
{
    return 2.0f / (1.0f + exp(-2 * x)) - 1;
}

class TargetBox
{
private:
    float GetWidth() { return (x2 - x1); };
    float GetHeight() { return (y2 - y1); };

public:
    int x1;
    int y1;
    int x2;
    int y2;

    int category;
    float score;

    float area() { return GetWidth() * GetHeight(); };
};

float IntersectionArea(const TargetBox &a, const TargetBox &b)
{
    if (a.x1 > b.x2 || a.x2 < b.x1 || a.y1 > b.y2 || a.y2 < b.y1)
    {
        // no intersection
        return 0.f;
    }

    float inter_width = std::min(a.x2, b.x2) - std::max(a.x1, b.x1);
    float inter_height = std::min(a.y2, b.y2) - std::max(a.y1, b.y1);

    return inter_width * inter_height;
}

bool scoreSort(TargetBox a, TargetBox b)
{
    return (a.score > b.score);
}

int nmsHandle(std::vector<TargetBox> &src_boxes, std::vector<TargetBox> &dst_boxes)
{
    std::vector<int> picked;

    sort(src_boxes.begin(), src_boxes.end(), scoreSort);

    for (int i = 0; i < src_boxes.size(); i++)
    {
        int keep = 1;
        for (int j = 0; j < picked.size(); j++)
        {
            // 交集
            float inter_area = IntersectionArea(src_boxes[i], src_boxes[picked[j]]);
            // 并集
            float union_area = src_boxes[i].area() + src_boxes[picked[j]].area() - inter_area;
            float IoU = inter_area / union_area;

            if (IoU > 0.45 && src_boxes[i].category == src_boxes[picked[j]].category)
            {
                keep = 0;
                break;
            }
        }

        if (keep)
        {
            picked.push_back(i);
        }
    }

    for (int i = 0; i < picked.size(); i++)
    {
        dst_boxes.push_back(src_boxes[picked[i]]);
    }

    return 0;
}

int fasetest_detect(cv::Mat srcImg, const char *model_bin, const char *model_param,  ncnn::Net &net, std::vector<TargetBox> &dst_boxes)
{
    int class_num = sizeof(class_names) / sizeof(class_names[0]);
    float thresh = 0.5;

    // int input_width = 352;
    // int input_height = 352;
    int input_width = 256;
    int input_height = 160;

    int img_width = srcImg.cols;
    int img_height = srcImg.rows;

    ncnn::Mat input = ncnn::Mat::from_pixels_resize(srcImg.data, ncnn::Mat::PIXEL_BGR,
                                                    srcImg.cols, srcImg.rows, input_width, input_height);

    const float mean_vals[3] = {0.f, 0.f, 0.f};
    const float norm_vals[3] = {1 / 255.f, 1 / 255.f, 1 / 255.f};
    input.substract_mean_normalize(mean_vals, norm_vals);

    ncnn::Extractor ex = net.create_extractor();
    // ex.set_num_threads(4);

    ex.input("input.1", input);

    ncnn::Mat output;
    // ex.extract("758", output);
    ex.extract("734", output);
    printf("output: %d, %d, %d\n", output.c, output.h, output.w);

    std::vector<TargetBox> target_boxes;

    for (int h = 0; h < output.h; h++)
    {
        for (int w = 0; w < output.h; w++)
        {
            int obj_score_index = (0 * output.h * output.w) + (h * output.w) + w;
            float obj_score = output[obj_score_index];

            int category;
            float max_score = 0.0f;
            for (size_t i = 0; i < class_num; i++)
            {
                int obj_score_index = ((5 + i) * output.h * output.w) + (h * output.w) + w;
                float cls_score = output[obj_score_index];
                if (cls_score > max_score)
                {
                    max_score = cls_score;
                    category = i;
                }
            }
            float score = pow(max_score, 0.4) * pow(obj_score, 0.6);

            if (score > thresh)
            {
                int x_offset_index = (1 * output.h * output.w) + (h * output.w) + w;
                int y_offset_index = (2 * output.h * output.w) + (h * output.w) + w;
                int box_width_index = (3 * output.h * output.w) + (h * output.w) + w;
                int box_height_index = (4 * output.h * output.w) + (h * output.w) + w;

                float x_offset = Tanh(output[x_offset_index]);
                float y_offset = Tanh(output[y_offset_index]);
                float box_width = Sigmoid(output[box_width_index]);
                float box_height = Sigmoid(output[box_height_index]);

                float cx = (w + x_offset) / output.w;
                float cy = (h + y_offset) / output.h;

                int x1 = (int)((cx - box_width * 0.5) * img_width);
                int y1 = (int)((cy - box_height * 0.5) * img_height);
                int x2 = (int)((cx + box_width * 0.5) * img_width);
                int y2 = (int)((cy + box_height * 0.5) * img_height);

                target_boxes.push_back(TargetBox{x1, y1, x2, y2, category, score});
            }
        }
    }

    nmsHandle(target_boxes, dst_boxes);
    return 0;
}

int draw_box(cv::Mat srcImg, std::vector<TargetBox> nms_boxes)
{

    for (size_t i = 0; i < nms_boxes.size(); i++)
    {
        TargetBox box = nms_boxes[i];
        printf("x1:%d y1:%d x2:%d y2:%d  %s:%.2f%%\n", box.x1, box.y1, box.x2, box.y2, class_names[box.category], box.score * 100);

        cv::rectangle(srcImg, cv::Point(box.x1, box.y1), cv::Point(box.x2, box.y2), cv::Scalar(0, 0, 255), 2);
        cv::putText(srcImg, class_names[box.category], cv::Point(box.x1, box.y1), cv::FONT_HERSHEY_SIMPLEX, 0.75, cv::Scalar(0, 255, 0), 2);
    }
    
    return 0;
}

int inference_img(const char *model_bin, const char *model_param, char *images_path)
{
    cv::Mat srcImg = cv::imread(images_path);

    ncnn::Net net;
    net.opt.use_vulkan_compute = true;
    net.opt.num_threads = 4;
    net.load_param(model_param);
    net.load_model(model_bin);
    printf("ncnn model load sucess...\n");

    double start = ncnn::get_current_time();

    std::vector<TargetBox> nms_boxes;
    fasetest_detect(srcImg, model_bin, model_param, net, nms_boxes);

    double end = ncnn::get_current_time();
    double time = end - start;
    printf("Time:%7.2f ms\n", time);

    draw_box(srcImg, nms_boxes);

    cv::imwrite("result.jpg", srcImg);
    return 0;
}

int inference_video(const char *model_bin, const char *model_param, char *video_path)
{
    cv::Mat frame;

    cv::VideoCapture cap(video_path);

    if (!cap.isOpened())
    {
        std::cerr << "Error: Unable to open video or RTSP stream: " << video_path << std::endl;
        return -1;
    }

    ncnn::Net net;
    net.opt.use_vulkan_compute = true;
    net.opt.num_threads = 4;
    net.load_param(model_param);
    net.load_model(model_bin);

    printf("ncnn model load sucess...\n");

    while (true)
    {
        printf("=========================\n");
        cap >> frame;
        if (frame.empty()) break; 

        double start = ncnn::get_current_time();

        std::vector<TargetBox> nms_boxes;
        fasetest_detect(frame, model_bin, model_param, net, nms_boxes);

        double end = ncnn::get_current_time();
        double time = end - start;
        // printf("Time:%7.2f ms\n", time);
        printf("Time:%7.2f fps/s\n", 1000/time);

        draw_box(frame, nms_boxes);

        cv::imshow("demo", frame);

        nms_boxes.clear();

        if (cv::waitKey(1) >= 0) break;
    }
    cap.release();
    cv::destroyAllWindows();
    return 0;
}

void help(char *name)
{
    printf("Run Options: \n");
    printf("%s <model.bin> <model.param> <images-dir>\n", name);
    exit(-1);
}

int main(int argc, char **argv)
{

    if (argc < 3)
    {
        help(argv[0]);
    }

    char *model_bin = argv[1];
    char *model_param = argv[2];
    char *video_path = argv[3];
    // char *images_path = argv[3];

    // inference_img(model_bin, model_param, images_path);
    inference_video(model_bin, model_param, video_path);

    return 0;
}