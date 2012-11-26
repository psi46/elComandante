/**
 *  \file replace_all.h
 *  \author Jochen Steinmann <jochen.steinmann@rwth-aachen.de>
 */

/**
 * \brief replace all
 * \param[in,out] text text where to replace
 * \param[in] fnd text which should be replaced
 * \param[in] rep text to replace with
 */
void replace_all(std::string& text,const std::string& fnd,const std::string& rep){
	size_t pos = text.find(fnd);
	while(pos!=std::string::npos){
		text.replace(pos,fnd.length(),rep);
		pos = text.find(fnd,pos+rep.length());
	}
}
