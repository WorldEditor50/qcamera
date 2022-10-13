#include "yolov4.h"
#include "cpu.h"

bool Yolov4::load(const std::string &modelType)
{
    if (modelType.empty()) {
        return false;
    }

    std::string paramFile = modelType + ".param";
    std::string modelFile = modelType + ".bin";
    int ret = yolov4.load_param(paramFile.c_str());
    if (ret != 0) {
        return false;
    }
    ret = yolov4.load_model(modelFile.c_str());
    if (ret != 0) {
        return false;
    }
    hasLoadModel = true;
    return true;
}

void Yolov4::detect(const cv::Mat &image, QVector<Object> &objects)
{
    if (hasLoadModel == false) {
        return;
    }
    int img_w = image.cols;
    int img_h = image.rows;
    ncnn::Mat in = ncnn::Mat::from_pixels_resize(image.data,
                                                 ncnn::Mat::PIXEL_RGBA2RGB,
                                                 img_w, img_h,
                                                 target_size, target_size);

    const float mean_vals[3] = {0, 0, 0};
    const float norm_vals[3] = {1 / 255.f, 1 / 255.f, 1 / 255.f};
    in.substract_mean_normalize(mean_vals, norm_vals);

    ncnn::Extractor ex = yolov4.create_extractor();

    ex.input("data", in);

    ncnn::Mat out;
    ex.extract("output", out);

    objects.clear();
    for (int i = 0; i < out.h; i++) {
        const float* values = out.row(i);

        Object object;
        object.label = values[0];
        object.prob = values[1];
        object.rect.x = values[2] * img_w;
        object.rect.y = values[3] * img_h;
        object.rect.width = values[4] * img_w - object.rect.x;
        object.rect.height = values[5] * img_h - object.rect.y;

        objects.push_back(object);
    }
    return;
}

void Yolov4::draw(cv::Mat &rgb, const QVector<Object> &objects)
{
    static const char* class_names[] = {"background", "person", "bicycle",
                                        "car", "motorbike", "aeroplane", "bus", "train", "truck",
                                        "boat", "traffic light", "fire hydrant", "stop sign",
                                        "parking meter", "bench", "bird", "cat", "dog", "horse",
                                        "sheep", "cow", "elephant", "bear", "zebra", "giraffe",
                                        "backpack", "umbrella", "handbag", "tie", "suitcase",
                                        "frisbee", "skis", "snowboard", "sports ball", "kite",
                                        "baseball bat", "baseball glove", "skateboard", "surfboard",
                                        "tennis racket", "bottle", "wine glass", "cup", "fork",
                                        "knife", "spoon", "bowl", "banana", "apple", "sandwich",
                                        "orange", "broccoli", "carrot", "hot dog", "pizza", "donut",
                                        "cake", "chair", "sofa", "pottedplant", "bed", "diningtable",
                                        "toilet", "tvmonitor", "laptop", "mouse", "remote", "keyboard",
                                        "cell phone", "microwave", "oven", "toaster", "sink",
                                        "refrigerator", "book", "clock", "vase", "scissors",
                                        "teddy bear", "hair drier", "toothbrush"
                                       };
    for (int i = 0; i < objects.size(); i++) {
        const Object& obj = objects[i];

        cv::rectangle(rgb, obj.rect, cv::Scalar(0, 255, 0));

        std::string text = std::string(class_names[obj.label]) + std::to_string( obj.prob * 100);
        int baseLine = 0;
        cv::Size label_size = cv::getTextSize(text, cv::FONT_HERSHEY_SIMPLEX, 0.5, 1, &baseLine);

        int x = obj.rect.x;
        int y = obj.rect.y - label_size.height - baseLine;
        if (y < 0) {
            y = 0;
        }
        if (x + label_size.width > rgb.cols) {
            x = rgb.cols - label_size.width;
        }

        cv::rectangle(rgb, cv::Rect(cv::Point(x, y), cv::Size(label_size.width, label_size.height + baseLine)),
                      cv::Scalar(0, 255, 0), 1);

        cv::putText(rgb, text, cv::Point(x, y + label_size.height),
                    cv::FONT_HERSHEY_SIMPLEX, 0.5, cv::Scalar(0, 255, 0));
    }
    return;
}

Yolov4::Yolov4():hasLoadModel(false)
{
    blob_pool_allocator.set_size_compare_ratio(0.f);
    workspace_pool_allocator.set_size_compare_ratio(0.f);
    blob_pool_allocator.clear();
    workspace_pool_allocator.clear();
    yolov4.opt.blob_allocator = &blob_pool_allocator;
    yolov4.opt.workspace_allocator = &workspace_pool_allocator;
    ncnn::set_cpu_powersave(2);
    ncnn::set_omp_num_threads(ncnn::get_big_cpu_count());
    yolov4.opt = ncnn::Option();
    yolov4.opt.num_threads = ncnn::get_big_cpu_count(); //You need to compile with libgomp for multi thread support
#ifdef Q_OS_ANDROID
    yolov4.opt.use_vulkan_compute = true; //You need to compile with libvulkan for gpu support
#else
    yolov4.opt.use_vulkan_compute = false;
#endif
    yolov4.opt.use_winograd_convolution = true;
    yolov4.opt.use_sgemm_convolution = true;
    yolov4.opt.use_fp16_packed = true;
    yolov4.opt.use_fp16_storage = true;
    yolov4.opt.use_fp16_arithmetic = true;
    yolov4.opt.use_packing_layout = true;
    yolov4.opt.use_shader_pack8 = false;
    yolov4.opt.use_image_storage = false;
    return;
}
