#include <Geode/Bindings.hpp>
#include <Geode/modify/GJLevelList.hpp>

#include "../utils.hpp"

using namespace geode::prelude;

class $modify(BIGJLevelList, GJLevelList) {
    void showListInfo() {
        std::string desc = getUnpackedDescription();

        std::stringstream ss;
        ss << (desc.empty() ? std::string("(No description provided)") : desc) << "\n\n";

        if(m_listID > 0) ss << "List ID: <cy>" << m_listID << "</c>\n";
        if(m_uploadDate > 0) ss << "Uploaded: <cy>" << TimeUtils::timestampToHumanReadable(m_uploadDate) << " ago</c> <cl>(" << TimeUtils::timeToString(m_uploadDate) << ")</c>\n";
        if(m_updateDate > 0) ss << "Updated: <cy>" << TimeUtils::timestampToHumanReadable(m_updateDate) << " ago</c> <cl>(" << TimeUtils::timeToString(m_updateDate) << ")</c>";

        FLAlertLayer::create(nullptr, "List Info", ss.str().c_str(), "OK", nullptr, 380.f)->show();
    }
};