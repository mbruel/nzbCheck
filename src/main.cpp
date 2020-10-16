#include "NzbCheck.h"
#include <QCoreApplication>

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);
    NzbCheck nzbCheck;
    if (nzbCheck.parseCommandLine(argc, argv))
    {
        int nbArticles = nzbCheck.parseNzb();
        if (nbArticles > 0 )
        {
            nzbCheck.checkPost();
            a.exec(); // start event loop
            return nzbCheck.nbMissingArticles();
        }
        else
            return nbArticles;
    }
    else
        return -1;
}
