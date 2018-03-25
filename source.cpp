#include <iostream>
#include <fstream>
#include <math.h>
#include <vector>
#include <list>
#include <functional>
#include <algorithm>
#include <string>
#include <sstream>
#include <memory>
#include <utility>

using namespace std;
using Unit = long long;

struct Coord
{
    Unit row{0};
    Unit col{0};
    Unit time{0};
    Coord& operator=(const Coord&c) = default;
};

using CoordCode = pair<Coord, Coord>;

const Coord& GetOrigin()
{
    static Coord origin{0,0,0};
    return origin;
}

const Coord& GetHorizon(){
    static Coord horizon{-1,-1,-1};
    return horizon;
}


ostream& operator << (ostream& o, const Coord& c){
    o << "(" << c.row << "," << c.col << "," << c.time << ")";
    return o;
}
ifstream& operator>>(ifstream& i, Coord& c){
    i >> c.row >> c.col;
    return i;
}

Unit ComputeDistance(const Coord& c1, const Coord& c2){
    if(c1.col == -1 && c1.row == -1 && c2.col == -1 && c2.row == -1)
        return 0;
    if(c1.col == -1 && c1.row == -1)
        return c2.col + c2.row;
    if(c2.col == -1 && c2.row == -1)
        return c1.col + c1.row;
    return abs(c1.row - c2.row)
    + abs(c1.col - c2.col)
    ;
}

Coord AtTarget(const Coord& dep, const Coord& target)
{
    if(target.time == -1)
        return dep;
    if(dep.time == -1)
        return dep;
    Coord result = target;
    result.time  = dep.time + target.time;
    return result;
}

bool ArriveOnTime(const Coord& dep, const Coord& target)
{
    if(target.time == -1)
        return true;
    auto atTarget = AtTarget(dep, target);
    if(atTarget.time == -1)
    {
        return false;
    }
    return atTarget.time <= target.time;
}

class Ride
{
    mutable Unit distance{-1};
    mutable bool bonusPossible{false};
    mutable bool assigned {false};
public:
    Unit id{-1};
    Unit indexDeparture{-1};
    Unit indexBestArrival{-1};
    const bool IsAssigned() const { return assigned;}
    void Assigned() const
    {
        assigned = true;
    }
    Coord dep;
    Coord arr;
    Unit DistanceOfRide() const{
        if(distance >= 0)
            return distance;
        distance = ComputeDistance(dep, arr);
        return distance;
    }
};

bool operator==(const Ride& r1, const Ride& r2)
{
    return r1.id == r2.id;
}

ostream& operator << (ostream& o, const Ride& r){
    const char* assigned = r.IsAssigned()?"assigned":"not assigned";
    o << r.id << " : " << r.dep << " - " << r.arr << " : " << r.DistanceOfRide() << ", " << assigned;
    return o;
}

ifstream& operator>>(ifstream& i, Ride& r){
    i >> r.dep >> r.arr >> r.dep.time >> r.arr.time;
    ++r.dep.time;
    ++r.arr.time;
    return i;
}


struct Header
{
    Unit rows{0};
    Unit cols{0};
    Unit fleet{0};
    Unit rides{0};
    Unit bonus{0};
    Unit maxTime{0};
};

ostream& operator << (ostream& o, const Header& h){
    o << h.rows << " " << h.cols << " " << h.fleet << " " << h.rides << " " << h.bonus << " " << h.maxTime;
    return o;
}
ifstream& operator>>(ifstream& i, Header& h){
    i >> h.rows >> h.cols >> h.fleet >> h.rides >> h.bonus >> h.maxTime;
    ++h.maxTime;
    return i;
}

using Rides = vector<Ride>;

struct InputData
{
    Header header;
    Rides rides;
};

ostream& operator << (ostream& o, const InputData& i){
    o << i.header << "\n";
    for(const auto& r : i.rides)
        o << r << "\n";
    return o;
}
ifstream& operator>>(ifstream& i, InputData& id){
    string line;
    i >> id.header;
    getline(i,line);
    id.rides.resize(id.header.rides);
    for(int j = 0; j < id.header.rides; ++j)
    {
        auto& ride = id.rides[j];
        ride.id = j;
        i >> ride;
        getline(i,line);
    }
    return i;
}

struct CheckRides
{
    InputData& inputData;
    CheckRides(InputData& id): inputData(id){}
    Unit Score(ifstream& input)
    {
        string line;
        Unit nbRides{0};
        Unit rideIndexValue {-1};
        Unit nbVehicle = 0;
        Unit score = 0;
        do
        {
            input >> nbRides;
            if(input.eof())
                return score;

            if(nbVehicle++ > inputData.header.fleet)
            {
                cout << "Nb vehicles used greater than fleet" << endl;
                return -1;
            }

            Coord currentPosition;
            for(Unit currentRideIndex = 0; currentRideIndex < nbRides; ++currentRideIndex)
            {
                rideIndexValue = -1;
                input >> rideIndexValue;
                if(rideIndexValue < 0 || rideIndexValue >= inputData.header.rides)
                {
                    nbVehicle--;
                    cout << "Invalid ride index : "
                        << rideIndexValue
                        << " at line "
                        << nbVehicle
                        << endl;
                    return -1;
                }

                auto& currentRide = inputData.rides[rideIndexValue];
                if(currentRide.IsAssigned())
                {
                    nbVehicle--;
                    cout << "Ride:"
                        << currentRide
                        << " is already assigned"
                        << " at line "
                        << nbVehicle
                        << endl;
                    return -1;
                }
                currentRide.Assigned();
                auto timeToArrival = currentPosition.time;
                timeToArrival += ComputeDistance(currentPosition, currentRide.dep);
                bool isOnTime = (timeToArrival <= currentRide.dep.time);

                currentPosition = currentRide.dep;
                timeToArrival = max(currentPosition.time, timeToArrival);
                timeToArrival += currentRide.DistanceOfRide();

                if( timeToArrival <= currentRide.arr.time)
                {
                    score += (isOnTime?inputData.header.bonus:0);
                    score += currentRide.DistanceOfRide();
                }

                currentPosition = currentRide.arr;
                currentPosition.time = timeToArrival;
            }
            getline(input,line);
        }
        while(!input.eof());
        return score;
    }
};

int main(int argc, char* argv[])
{
    if(argc < 1)
        return 0;

    InputData ip;
    string filePath = argv[1];
    ifstream file(filePath);
    file >> ip;

    filePath = argv[2];
    ifstream solution(filePath);

    CheckRides cr(ip);
    auto score = cr.Score(solution);
    cout << "Score is: " << score << endl;
    return 0;
}
