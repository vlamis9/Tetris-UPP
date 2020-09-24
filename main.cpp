#include <CtrlLib/CtrlLib.h>
#include <Core/Core.h>

#include <vector>
#include <set>
//#include <thread> //for delay if need
//#include <chrono>

#define emptyCell Color(150, 150, 150)

using namespace Upp;


class Figure 
{
	friend class Glass;
    friend class NextFigure;    
    
public:
		void setIndex(uint, uint);	
		Figure(uint, uint = 0, uint = 0);
		void paintFigure(Draw& w);
		void rotateColors(bool);
    	void makeRandomColors(); 
	
private:
    	Color fig[3];
    	uint m_W, m_x, m_y;  	
};

Figure::Figure(uint W, uint x, uint y): m_W(W), m_x(x), m_y(y)
{
	makeRandomColors();
}

void Figure::setIndex(uint x, uint y)
{
    m_x = x;
    m_y = y;     
}

void Figure::paintFigure(Draw& w)
{
	uint x = m_x;
    uint y = m_y;
    for (int ind = 0; ind < 3; ind++)
    {
        w.DrawRect(x * m_W + 1, y * m_W + 1, m_W - 1, m_W - 1, fig[ind]); 
        //w.DrawEllipse(x * m_W + 1, y * m_W + 1, m_W - 1, m_W - 1, fig[ind], Color(Black));       
        y++;
    }
}

void Figure::rotateColors(bool down)
{
    if(down)
    {
        std::swap(fig[0], fig[1]);
        std::swap(fig[2], fig[0]);
    }
    else
    {
        std::swap(fig[2], fig[1]);
        std::swap(fig[0], fig[2]);
    }
}

void Figure::makeRandomColors()
{
    Color colors[3]{Color(Red), Color(Green), Color(Blue)};
    for (auto& elem : fig)
    {
        elem = colors[rand() % 3];
    }
}

class NextFigure : public Ctrl
{	
		
public:	
		NextFigure(); 
		virtual void Paint(Draw& w) override;
		void slotNewFigure(Figure* next);		
	
private:
		Figure* nextFig;
		uint W;	
};

NextFigure::NextFigure()
{	
	W = 30;
	nextFig = nullptr;
}

void NextFigure::slotNewFigure(Figure* next)
{
    nextFig = next;    
    Refresh();
}

void NextFigure::Paint(Draw& w)
{
    if (nextFig)
    {        
        int x = 0;
        int y = 0;
        for (int ind = 0; ind < 3; ind++)
        {
            w.DrawRect(x * W + 1, y * W + 1, W - 1, W - 1, nextFig->fig[ind]);            
            //w.DrawEllipse(x * W + 1, y * W + 1, W - 1, W - 1, nextFig->fig[ind], Color(Black));                 
            y++;
        }
    }
}

class TTT;

class Glass : public Ctrl
{	
public:
		friend class TTT;
		typedef Glass CLASSNAME;
	
		static const uint W = 20;		
		Glass();
		~Glass();
		void clearGlass();	
		void slotGlassInit();
		void slotNewGame();
		void acceptColors(uint, uint);
    	bool analyzeAll();
    	void analyzeHor();
    	void analyzeVer();
    	void boom();
   		bool doTheJob();
   		
		virtual void Paint(Draw& w) override;			
		virtual bool Key(dword, int) override;
		void timerEvent();
		
		void slotFillTest();
		void slotTest();				
		
private:
		uint m_rows;
    	uint m_columns;
    	bool gameOn;
    	int score;
    	int finishScore;
		std::vector<std::vector<Color>> glassArray;    
    	Figure* cur, *next;    	
    	uint timerS = 500;
    	bool swapped;
    	std::vector<std::vector<Color>> vectorHor;
    	uint edge;
    	std::vector<std::pair<uint, uint>> superVector;
    	
    	bool signalNext;  
    	bool signalScore;
    	bool reset;	
};

Glass::Glass() 
{ 
	m_rows = 20;
	m_columns = 10;
	gameOn = false;
    glassArray = std::vector<std::vector<Color>>{};
        
    score = 0;
    finishScore = 0;
    slotGlassInit();
    cur = new Figure(W);
    next = new Figure(W);

    edge = m_rows - 1;
    swapped = false;	
    
    signalNext = false;
    signalScore = false; 
    reset = false;   
}

Glass::~Glass()
{
	delete cur;
    delete next;
}

void Glass::clearGlass()
{
    for(int i = 0; i < glassArray.size(); i++)
    {
        glassArray[i] = std::vector<Color>(m_columns, emptyCell);
	}
	score = 0;
}

void Glass::slotGlassInit()
{
    glassArray.resize(m_rows);
    for(int i = 0; i < glassArray.size(); i++)
    {
        glassArray[i] = std::vector<Color>(m_columns, emptyCell);
	}	
}

void Glass::Paint(Draw& w) 
{	
	for (uint i = 0; i < m_rows; i++)
    {
        for (uint j = 0; j <m_columns; j++)
        {
           	w.DrawRect(j * W + 1, i * W + 1, W - 1, W - 1, glassArray[i][j]);  
           	//w.DrawEllipse(j * W + 1, i * W + 1, W - 1, W - 1, glassArray[i][j], 1, Color(Black));           	      	         	          
        }        
    }
    if(gameOn)
    {
        cur->paintFigure(w);        
    }		
}

void Glass::slotNewGame()
{
    gameOn = true;
    clearGlass();
    cur->setIndex(m_columns / 2, 0);
    next->setIndex(0, 0);    
    
    SetTimeCallback(-timerS, THISBACK(timerEvent));
	
	signalNext = true;
	reset = true;
    RefreshParentLayout();
    signalNext = false;  
    reset = false;  
    
    Refresh();
}

void Glass::timerEvent()
{	
    if((cur->m_y + 3 != m_rows) && ((glassArray[cur->m_y + 3][cur->m_x] == emptyCell)))
    {
        cur->setIndex(cur->m_x, cur->m_y + 1);
        Refresh();
    }
    else
    {
    	this->acceptColors(cur->m_y, cur->m_x);   
    }
}

bool Glass::Key(dword key, int count)
{		
	if(gameOn) 
    {      
		switch(key)
        {
            case K_LEFT:
                if((cur->m_x > 0) && (glassArray[cur->m_y + 2][cur->m_x - 1] == emptyCell))
                {
                    cur->setIndex(cur->m_x - 1, cur->m_y);                    
                    Refresh();                    
                }
                break;
            case K_RIGHT:
                if((cur->m_x < (this->GetRect().GetWidth() - 1) / W) && (glassArray[cur->m_y + 2][cur->m_x + 1] == emptyCell))
                {
                    cur->setIndex(cur->m_x + 1, cur->m_y);
                    Refresh();
                }
                break;
            case K_DOWN:
                cur->rotateColors(true);
                Refresh();
                break;
            case K_UP:
                cur->rotateColors(false);
                Refresh();
                break;
            case K_SPACE:                
                for (uint index = cur->m_y + 2; index < m_rows; index++)
                {
                    if(glassArray[index][cur->m_x] == emptyCell)
                    {
                        cur->setIndex(cur->m_x, index-2);
                        continue;
                    }
                    else
                    {
                        break;
                    }
                }
                Refresh();
                break;            
        }
        return true;
    }
    else return false;	
}


void Glass::acceptColors(uint , uint)
{    
    this->KillTimeCallback();    
    
	if (glassArray[cur->m_y + 2][cur->m_x] != emptyCell)
    {    
    	finishScore = score;    
        for(int i = 0; i < m_rows; i++)
        {
            std::fill(glassArray[i].begin(), glassArray[i].end(), Color(Black));
            
            Sync();
            Refresh();
            Sleep(150);
            Sync();
        }

        clearGlass();
        Refresh();
        gameOn = false;
        PromptOK("You lose this game. You can try again and maybe you'll get more points."
        		 " Now your score is " + IntStr(finishScore));    
    }
    if (gameOn)
    {

    //Save blocks in Array
    uint y = cur->m_y;
    uint x = cur->m_x;
    for (int count = 0; count < 3; count++)
    {
        glassArray[y][x] = cur->fig[count];
        y++;
    }

    edge = edge > cur->m_y ? cur->m_y : edge; //Top(est) Y, cut for analyze

    for (uint i = edge; i < m_rows; i++)
    {
        vectorHor.push_back(glassArray[i]);        
    }

    std::swap(cur, next);
    cur->setIndex(m_columns / 2, 0);
    next->makeRandomColors();
    next->setIndex(0, 0);

    while (analyzeAll())
    {
        superVector.clear();
        vectorHor.resize(0);
        for (uint i = edge; i < m_rows; i++)
        {
            vectorHor.push_back(glassArray[i]);
        }
    }    

    vectorHor.clear();
    superVector.clear();
    
    signalNext = true;
    RefreshParentLayout();
    signalNext = false;

    SetTimeCallback(-timerS, THISBACK(timerEvent));
    }
}

bool Glass::analyzeAll()
{
    analyzeHor();
    analyzeVer();
    doTheJob();

    signalScore = true;
    RefreshParentLayout();
    signalScore = false;
    
    return swapped;
}

bool Glass::doTheJob()
{
    swapped = false;

    //sort and remove duplicates for counting points
    std::sort(superVector.begin(), superVector.end());
    auto it = std::unique(superVector.begin(), superVector.end());
    superVector.resize(std::distance(superVector.begin(), it));

    score += superVector.size(); 

    auto maxY = std::max_element(superVector.begin(), superVector.end()); //lowest Y
    std::set<uint> setX; //how much indexes by X
    for (auto& pair : superVector)
    {
        setX.insert(pair.second);
    }

    //boom - destroy the same seq
    boom();
    
    //move blocks down 
    for (const auto& indX : setX)
    {
        int indY = maxY->first;
        for (int y = maxY->first; y - 1 >= 0;)
        {
            if (glassArray[y][indX] != emptyCell)
            {
                if (y - 1 == 0) break;
                else
                {
                    y--;
                    continue;
                }
            }
            else
            {
                indY = y;
                while ((glassArray[y][indX] == emptyCell) && (y > 0))
                {
                    y--;
                }
                std::swap(glassArray[y][indX], glassArray[indY][indX]);

                swapped = true;
                                
                indY--;
                y = indY;
                continue;
            }
        }
        Refresh();
    }
    return swapped;
}


void Glass::analyzeHor()
{    
    std::vector<std::pair<uint, uint>> vecTmp;
    uint indY = edge;
    for (auto& vector : vectorHor)
    {
        uint j = 0;
        uint counter = 1;

        bool yes = true;
        while (j + 1 < m_columns)
        {
            vecTmp.push_back(std::make_pair(indY, j));
            while (yes)
            {
                if ((vector[j] == vector[j + 1]) && (vector[j] != emptyCell))
                {
                    counter++;
                    j++;
                    vecTmp.push_back(std::make_pair(indY, j));
                    if (j == m_columns-1)
                        yes = false;
                }
                else (yes = false);
            }
            if (counter >= 3)
            {
                for (auto& elem : vecTmp)
                {
                    superVector.push_back(elem);
                }                
            }
            counter = 1;
            vecTmp.clear();
            j++;
            yes = true;
        }
        indY++;
    }
}

void Glass::analyzeVer()
{
    std::vector<std::pair<int, int>> vecTmp;
    for (uint x = 0; x < m_columns; x++)
    {
        for (uint y = m_rows - 1; y > m_rows - vectorHor.size();)
        {
            uint counter = 1;
            bool yes = true;
            vecTmp.push_back(std::make_pair(y, x));
            while (yes)
            {
                if ((glassArray[y][x] == glassArray[y-1][x]) && (glassArray[y][x] != emptyCell))
                {
                    counter++;
                    y--;
                    vecTmp.push_back(std::make_pair(y, x));
                    if (y == m_rows - vectorHor.size())
                    {
                        yes = false;                        
                    }
                }
                else (yes = false);
            }
            if (counter >= 3)
            {
                for (auto& elem : vecTmp)
                {
                    superVector.push_back(elem);
                }
            }
            counter = 1;
            vecTmp.clear();
            if (y == m_rows - vectorHor.size()) break;
            y--;
            yes = true;
        }
    }
}

void Glass::boom()
{
    for (auto& elem : superVector)
    {
        glassArray[elem.first][elem.second] = Color(Yellow);               
    }
    
    Sync(); //need thing    
    Refresh(); 
    //std::this_thread::sleep_for(std::chrono::milliseconds(300));
    Sleep(300); 
    Sync(); //need thing   
    
    for (auto& elem : superVector)
    {
        glassArray[elem.first][elem.second] = emptyCell;
    }
    Refresh();
}


void Glass::slotFillTest()
{    
    gameOn = false;
    clearGlass();    
    Refresh();    
    Color colors[3]{Color(Red), Color(Green), Color(Blue)};
    for (auto& vector : glassArray)
    {
        for (auto& elem : vector)
        {
            elem = colors[rand() % 3];
        }
    }
    edge = 0;
    score = 0;
    
    signalScore = true;
    RefreshParentLayout();
    signalScore = false;    
    Refresh();
}

void Glass::slotTest()
{
    vectorHor = glassArray;
    while (analyzeAll())
    {
        superVector.clear();
        vectorHor.resize(0);
        for (uint i = edge; i < m_rows; i++)
        {
            vectorHor.push_back(glassArray[i]);
        }
    }
}


class Score : public TextCtrl
{	
		
public:	
		Score(); 
		virtual void Paint(Draw& w) override;	
		void setScore(int);		
	
private:
		int score;	
};

Score::Score(): score(0) {}

void Score::Paint(Draw& w)
{
	w.DrawText(0, 0, Value(score).ToString(), Arial(40).Bold());	
}

void Score::setScore(int val)
{
	score = val;
	Refresh();
}


#define LAYOUTFILE <TTT/TTT.lay>
#include <CtrlCore/lay.h>

class TTT : public WithMainLayout<TopWindow> 
{
public:
	typedef TTT CLASSNAME;
	TTT();
	virtual bool Key(dword, int) override;	
	virtual void Layout() override;
	void buttonExitClicked();
	void slotPauseDisabled();
	void slotPauseEnabled();
	void slotContinueDisabled();
	void slotContinueEnabled();
};

TTT::TTT()
{
	CtrlLayout(*this, "Window title");
	buttonExit <<= THISBACK(buttonExitClicked);
	
	buttonStart.WhenAction = [=]{glass.slotNewGame(); slotPauseEnabled();};
	buttonPause.WhenAction = [=]{slotPauseDisabled(); slotContinueEnabled();};
	buttonContinue.WhenAction = [=]{slotContinueDisabled(); slotPauseEnabled();};
	
	buttonFill.WhenAction = [=]{glass.slotFillTest();};
	buttonTest.WhenAction = [=]{glass.slotTest();};
	buttonPause.Disable();
	buttonContinue.Disable();
	
	glass.SetRect(this->glass.GetRect().TopLeft().x, 
						this->glass.GetRect().TopLeft().y, 20 * 20, 20 * 20);	
						
	this->SetRect(0, 0, 450, 500);
}

bool TTT::Key(dword key, int x)
{
	this->glass.SetFocus();
	this->glass.Key(key, x);
	return true;
}


void TTT::Layout()
{
	if (glass.signalNext == true)
	{		
		myctrl.slotNewFigure(glass.next);			
	}
	if (glass.signalScore == true)
	{
		scoreView.setScore(glass.score);
	}
	if (glass.reset == true)
	{
		slotPauseDisabled();
		slotContinueDisabled();
		scoreView.setScore(0);
	}
}

void TTT::buttonExitClicked()
{
	if(PromptOKCancel("Exit MyApp?"))
		Break();	
}

void TTT::slotPauseDisabled()
{
    buttonPause.Disable();    
    glass.KillTimeCallback();
}

void TTT::slotPauseEnabled()
{
    buttonPause.Enable();
}

void TTT::slotContinueDisabled()
{
    buttonContinue.Disable();    
    glass.SetTimeCallback(-(glass.timerS), [=]{glass.timerEvent();});    
}

void TTT::slotContinueEnabled()
{
    buttonContinue.Enable();
}

GUI_APP_MAIN
{
	TTT().Run();	
}