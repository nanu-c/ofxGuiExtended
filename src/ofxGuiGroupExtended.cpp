#include "ofxGuiGroupExtended.h"
#include "ofGraphics.h"
using namespace std;

ofxGuiGroupExtended::ofxGuiGroupExtended(){
    _bVertical = true;
    _bUseHeader = true;
    _bAllowMultiple = true;
}

ofxGuiGroupExtended::ofxGuiGroupExtended(const ofParameterGroup & parameters, string filename, float x, float y)
    :ofxGuiGroup(parameters, filename, x, y){
    _bVertical = true;
    _bUseHeader = true;
    _bAllowMultiple = true;
}

ofxGuiGroupExtended * ofxGuiGroupExtended::setup(string collectionName, string filename, float x, float y){
    ofxGuiGroup::setup(collectionName,filename,x,y);
    return this;
}

ofxGuiGroupExtended * ofxGuiGroupExtended::setup(const ofParameterGroup & _parameters, string filename, float x, float y){
    ofxGuiGroup::setup(_parameters,filename,x,y);
    return this;
}

void ofxGuiGroupExtended::add(ofxBaseGui * element){
    collection.push_back( element );

    if(_bVertical || collection.size() == 1) {

        element->setPosition(b.x, b.y + b.height  + spacing);

        b.height += element->getHeight() + spacing;
        if(b.width-1 > element->getWidth() && _bVertical) {
            element->setSize(b.width-1, element->getHeight());
        }
    }
    else {
        ofRectangle last_shape = collection[collection.size()-2]->getShape();
        element->setPosition(last_shape.x + last_shape.getWidth() + spacing, last_shape.y);
        b.width = element->getPosition().x + element->getWidth() - b.x + spacing;
    }

    if(b.width-1 < element->getWidth()) {
        element->setSize(b.width-1, element->getHeight());
    }

    element->unregisterMouseEvents();

    ofxGuiGroupExtended * subgroup = dynamic_cast<ofxGuiGroupExtended*>(element);
    if(subgroup!=NULL){
        subgroup->filename = filename;
        subgroup->parent = this;
        subgroup->scaleWidthElements(.98);
    }else{
        if(parent!=NULL){
            element->setSize(b.width*.98,element->getHeight());
            element->setPosition(b.x + b.width-element->getWidth(),element->getPosition().y);
        }
    }

    parameters.add(element->getParameter());

    setNeedsRedraw();
}

void ofxGuiGroupExtended::setWidthElements(float w){
    if((int)b.width != (int)w) {
        scaleWidthElements(w / b.width);
    }
}

void ofxGuiGroupExtended::scaleWidthElements(float factor){

    float w = b.getWidth()*factor;

    if(_bVertical) {
        for(int i=0;i<(int)collection.size();i++){
            collection[i]->setSize(w,collection[i]->getHeight());
            collection[i]->setPosition(b.x + b.width-w,collection[i]->getPosition().y);
            ofxGuiGroupExtended * subgroup = dynamic_cast<ofxGuiGroupExtended*>(collection[i]);
            if(subgroup!=NULL){
                subgroup->scaleWidthElements(factor);
            }
        }
    }
    else {
        float x = spacing;
        for(int i=(int)collection.size()-1;i>=0;i--){
            float w_i = collection[i]->getWidth()*factor;
            collection[i]->setSize(w_i,collection[i]->getHeight());
            x += w_i+spacing;
            collection[i]->setPosition(b.x + b.width-x,collection[i]->getPosition().y);
            ofxGuiGroupExtended * subgroup = dynamic_cast<ofxGuiGroupExtended*>(collection[i]);
            if(subgroup!=NULL){
                subgroup->scaleWidthElements(factor);
            }
        }
    }

    sizeChangedCB();
    setNeedsRedraw();

}

void ofxGuiGroupExtended::clear(){
    collection.clear();
    parameters.clear();
    b.height = spacing + spacingNextElement ;
    if(_bUseHeader) {
        b.height += header;
    }
    sizeChangedCB();
}

bool ofxGuiGroupExtended::mouseMoved(ofMouseEventArgs & args){
    return ofxGuiGroup::mouseMoved(args);
}


bool ofxGuiGroupExtended::mousePressed(ofMouseEventArgs & args){
    if(setValue(args.x, args.y, true)){
        return true;
    }
    if( bGuiActive ){
        ofMouseEventArgs a = args;
        for(int i = 0; i < (int)collection.size(); i++){
            ofxMinimalToggle* toggle = dynamic_cast<ofxMinimalToggle*>(collection[i]);
            if(toggle && !_bAllowMultiple) {
                if(processToggles(toggle, a)) return true;
            }
            else {
                if(collection[i]->mousePressed(a)) {
                    return true;
                }
            }
        }
    }
    return false;
}

bool ofxGuiGroupExtended::processToggles(ofxMinimalToggle* toggle, ofMouseEventArgs a) {
    if(!_bAllowMultiple) {
        if(!toggle->getParameter().cast<bool>().get()) {
            if(toggle->mousePressed(a)) {
                deactivateAllOtherToggles(toggle);
                return true;
            }
        }
    }
    return false;
}

bool ofxGuiGroupExtended::mouseDragged(ofMouseEventArgs & args){
    if(setValue(args.x, args.y, false)){
        return true;
    }
    if( bGuiActive ){
        ofMouseEventArgs a = args;
        for(int i = 0; i < (int)collection.size(); i++){
            if(collection[i]->mouseDragged(a)) return true;
        }
    }
    return false;
}

bool ofxGuiGroupExtended::mouseReleased(ofMouseEventArgs & args){
    return ofxGuiGroup::mouseReleased(args);
}

bool ofxGuiGroupExtended::mouseScrolled(ofMouseEventArgs & args){
    return ofxGuiGroup::mouseScrolled(args);
}

void ofxGuiGroupExtended::generateDraw(){
    border.clear();
    border.setFillColor(ofColor(thisBorderColor));
    border.setFilled(true);
    border.rectangle(b.x,b.y+ spacingNextElement,b.width+1,b.height);

    if(_bUseHeader) {
        headerBg.clear();
        headerBg.setFillColor(thisHeaderBackgroundColor);
        headerBg.setFilled(true);
        headerBg.rectangle(b.x,b.y + spacingNextElement, b.width+1, header);

        textMesh = getTextMesh(getName(), textPadding + b.x, header / 2 + 4 + b.y+ spacingNextElement);
        if(minimized){
            textMesh.append(getTextMesh("+", b.width-textPadding-8 + b.x, header / 2 + 4+ b.y+ spacingNextElement));
        }else{
            textMesh.append(getTextMesh("-", b.width-textPadding-8 + b.x, header / 2 + 4 + b.y+ spacingNextElement));
        }
    }

}

void ofxGuiGroupExtended::render(){
    border.draw();
    if(_bUseHeader) {
        headerBg.draw();
    }

    ofBlendMode blendMode = ofGetStyle().blendingMode;
    if(blendMode!=OF_BLENDMODE_ALPHA){
        ofEnableAlphaBlending();
    }
    ofColor c = ofGetStyle().color;

    if(_bUseHeader) {
        ofSetColor(thisTextColor);

        bindFontTexture();
        textMesh.draw();
        unbindFontTexture();
    }

    if(!minimized){
        for(int i = 0; i < (int)collection.size(); i++){
            collection[i]->draw();
        }
    }

    ofSetColor(c);
    if(blendMode!=OF_BLENDMODE_ALPHA){
        ofEnableBlendMode(blendMode);
    }
}

bool ofxGuiGroupExtended::setValue(float mx, float my, bool bCheck){

    if( !isGuiDrawing() ){
        bGuiActive = false;
        return false;
    }


    if( bCheck ){
        if( b.inside(mx, my) ){
            bGuiActive = true;

            ofRectangle minButton(b.x+b.width-textPadding*3,b.y,textPadding*3,header);
            if(minButton.inside(mx,my)){
                minimized = !minimized;
                if(minimized){
                    minimize();
                }else{
                    maximize();
                }
                return true;
            }
        }
    }

    return false;
}

void ofxGuiGroupExtended::minimize(){
    minimized=true;
    b.height = spacing + spacingNextElement + 1 /*border*/;
    if(_bUseHeader) {
        b.height += header;
    }
    if(parent) parent->sizeChangedCB();
    setNeedsRedraw();
}

void ofxGuiGroupExtended::maximize(){
    minimized=false;
    if(_bVertical) {
        for(int i=0;i<(int)collection.size();i++){
            b.height += collection[i]->getHeight() + spacing;
        }
    }
    else {
        float max_h = 0;
        for(int i=0;i<(int)collection.size();i++){
            if(max_h < collection[i]->getHeight()) {
                max_h = collection[i]->getHeight();
            }
        }
        b.height += max_h+spacing;
    }
    if(parent) parent->sizeChangedCB();
    setNeedsRedraw();
}

void ofxGuiGroupExtended::minimizeAll(){
    for(int i=0;i<(int)collection.size();i++){
        ofxGuiGroupExtended * group = dynamic_cast<ofxGuiGroupExtended*>(collection[i]);
        if(group)group->minimize();
    }
}

void ofxGuiGroupExtended::maximizeAll(){
    for(int i=0;i<(int)collection.size();i++){
        ofxGuiGroupExtended * group = dynamic_cast<ofxGuiGroupExtended*>(collection[i]);
        if(group)group->maximize();
    }
}

void ofxGuiGroupExtended::sizeChangedCB(){
    float x,y;
    if(parent){
        x = b.x + spacing + spacingNextElement;
        y = b.y + spacing + spacingNextElement;
    }else{
        x = b.x + spacing;
        y = b.y + spacing;

    }
    if(_bUseHeader) {
        y += header;
    }

    if(_bVertical) {
        for(int i=0;i<(int)collection.size();i++){
            collection[i]->setPosition(collection[i]->getPosition().x,y + spacing);
            y += collection[i]->getHeight()+spacing;
        }
        b.height = y - b.y;
    }
    else {
        x -= spacing;
        float max_h = 0;
        for(int i=0;i<(int)collection.size();i++){

            collection[i]->setPosition(x,y + spacing);

            x += collection[i]->getWidth() + spacing;
            if(max_h < collection[i]->getHeight()) {
                max_h = collection[i]->getHeight();
            }
        }
        y += max_h+spacing;
        x += spacing;
        b.height = y - b.y;
        b.width = x - b.x;
    }

    if(parent) parent->sizeChangedCB();
    setNeedsRedraw();
}

void ofxGuiGroupExtended::setAlignHorizontal() {
    _bVertical = false;
}

void ofxGuiGroupExtended::setAlignVertical() {
    _bVertical = true;
}

bool ofxGuiGroupExtended::isAlignedVertial() {
    return _bVertical;
}

void ofxGuiGroupExtended::showHeader(bool show) {
    if(show == false) {
        if(minimized)
            maximize();
    }
    _bUseHeader = show;
    sizeChangedCB();
    setNeedsRedraw();

}

void ofxGuiGroupExtended::allowMultipleActiveToggles(bool allow) {
    _bAllowMultiple = allow;
}

bool ofxGuiGroupExtended::setActiveToggle(ofxMinimalToggle* toggle) {
    if(!toggle->getParameter().cast<bool>().get()) {
        toggle->setParameter(true);
        deactivateAllOtherToggles(toggle);
        return true;
    }
    return false;
}

bool ofxGuiGroupExtended::setActiveToggle(int index) {
    if(ofxMinimalToggle* toggle = dynamic_cast<ofxMinimalToggle*>(collection[index])) {
        return setActiveToggle(toggle);
    }
    else {
        ofLogError("ofxGuiGroupExtended", "cannot activate control " + ofToString(index) + " because it's no ofxMinimalToggle.");
        return false;
    }
}

void ofxGuiGroupExtended::deactivateAllOtherToggles(ofxMinimalToggle *toggle) {
    if(!_bAllowMultiple) {
        for(int j = 0; j < (int)collection.size(); j++){
            if(ofxMinimalToggle* t = dynamic_cast<ofxMinimalToggle*>(collection[j])) {
                if(t != toggle) {
                   t->setParameter(false);
                }
                else {
                    active_toggle_index = j;
                }
            }
        }
    }
}

int ofxGuiGroupExtended::getActiveToggleIndex() {
    return active_toggle_index;
}

float ofxGuiGroupExtended::getContentHeight() {
    float h = b.height - spacing;
    if(_bUseHeader) {
       h -= header;
    }
    if(parent) {
        h -= spacingNextElement;
    }
    return h;
}
