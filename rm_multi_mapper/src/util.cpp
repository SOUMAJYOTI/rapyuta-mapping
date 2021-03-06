#include <util.h>

using Poco::URIStreamOpener;
using Poco::StreamCopier;
using Poco::Path;
using Poco::URI;
using Poco::Exception;
using Poco::Net::HTTPStreamFactory;
using namespace std;

cv::Mat util::loadFromURL(string url)
{
    //Don't register the factory more than once
    if(!factoryLoaded){
        HTTPStreamFactory::registerFactory();
        factoryLoaded = true;
    }

    //Specify URL and open input stream
    URI uri(url);
    std::auto_ptr<std::istream> pStr(URIStreamOpener::defaultOpener().open(uri));

    //Copy image to our string and convert to cv::Mat
    string str;
    StreamCopier::copyToString(*pStr.get(), str);
    vector<char> data( str.begin(), str.end() );
    cv::Mat data_mat(data);
    cv::Mat image(cv::imdecode(data_mat,1));
    return image;
}
    
cv::Mat util::stringtoMat(string file)
{
    cv::Mat image;

    if(file.compare(file.size()-4,4,".gif")==0)
    {
        cerr<<"UNSUPPORTED_IMAGE_FORMAT";
        return image;
    }

    else if(file.compare(0,7,"http://")==0)  // Valid URL only if it starts with "http://"
    {
        image = loadFromURL(file);
        if(!image.data)
            cerr<<"INVALID_IMAGE_URL";
        return image;
    }
    else
    {
        image = cv::imread(file,1); // Try if the image path is in the local machine
        if(!image.data)
            cerr<<"IMAGE_DOESNT_EXIST";
        return image;
    }
}


sql::ResultSet* util::sql_query(std::string query) {
    try {
        sql::Driver *driver;
        sql::Connection *con;
        sql::PreparedStatement *pstmt;
        sql::ResultSet *res;

        /* Create a connection */
        driver = get_driver_instance();
        con = driver->connect("tcp://127.0.0.1:3306", "root", "123456");
        /* Connect to the MySQL test database */
        con->setSchema("panorama");

        /* Select in ascending order */
        pstmt = con->prepareStatement(query);
        res = pstmt->executeQuery();
        
        delete pstmt;
        delete con;
        return res;

    } catch (sql::SQLException &e) {
        std::cout << "# ERR: SQLException in " << __FILE__;
        std::cout << "(" << __FUNCTION__ << ") on line " 
        << __LINE__ <<std::endl;
        std::cout << "# ERR: " << e.what();
        std::cout << " (MySQL error code: " << e.getErrorCode();
        std::cout << ", SQLState: " << e.getSQLState() << " )" << std::endl;
        return NULL;
    }
}


void util::load_mysql(std::vector<std::pair<Sophus::SE3f, Eigen::Vector3f> > & positions) {

    sql::ResultSet *res;
    res = sql_query("SELECT * FROM positions");

    while (res->next())
    {
        Eigen::Quaternionf q;
        Eigen::Vector3f t;
		Eigen::Vector3f intrinsics;
        q.coeffs()[0] = res->getDouble("q0");
        q.coeffs()[1] = res->getDouble("q1");
        q.coeffs()[2] = res->getDouble("q2");
        q.coeffs()[3] = res->getDouble("q3");
        t[0] = res->getDouble("t0");
        t[1] = res->getDouble("t1");
        t[2] = res->getDouble("t2");
        intrinsics[0] = res->getDouble("int0");
        intrinsics[1] = res->getDouble("int1");
        intrinsics[2] = res->getDouble("int2");
        positions.push_back(std::make_pair(Sophus::SE3f(q, t), intrinsics));
    }

    delete res;

}


void util::load(const std::string & dir_name, std::vector<color_keyframe::Ptr> & frames) {

    std::vector<std::pair<Sophus::SE3f, Eigen::Vector3f> > positions;

    load_mysql(positions);

    std::cerr << "Loaded " << positions.size() << " positions" << std::endl;

    for (size_t i = 0; i < positions.size(); i++) {
        cv::Mat rgb = stringtoMat(
		        dir_name + "/rgb/" + boost::lexical_cast<std::string>(i)
				        + ".png");
        cv::Mat depth = stringtoMat(
		        dir_name + "/depth/" + boost::lexical_cast<std::string>(i)
				        + ".png");

        cv::Mat gray;
        cv::cvtColor(rgb, gray, CV_RGB2GRAY);

        color_keyframe::Ptr k(
		        new color_keyframe(rgb, gray, depth, positions[i].first,
				        positions[i].second));
        frames.push_back(k);
    }

}


