#ifndef RUNTIMESIZEDOBJECTSSTACK_H
#define RUNTIMESIZEDOBJECTSSTACK_H

#include<vector>
#include<cmath>
#include<cstring>
#include<cstdlib>
#include<cstdint>

template <typename T1, typename T2, typename T3> inline constexpr
bool IsInSegment (T1 num_arg, T2 left_arg, T3 right_arg)
{
    return (num_arg>=left_arg) && (num_arg<=right_arg);
}
inline void SafeMemCopy (uint8_t* dest_arg, uint8_t* src_arg, size_t bytesMove_arg)
{
    if (IsInSegment(dest_arg, src_arg, src_arg+bytesMove_arg )
        &&IsInSegment(dest_arg+bytesMove_arg, src_arg, src_arg+bytesMove_arg) )
        memmove((void*)dest_arg, (void*)src_arg, bytesMove_arg);
    else
        memcpy( (void*)dest_arg, (void*)src_arg, bytesMove_arg  );
    //���� ���� ����������� - ����� memmove, ����� - memcpy
}

inline int DivAndCeil (int const a, int const b)
{
    div_t res = div(a,b);
    return (res.rem!=0)? (res.quot+1) : (res.quot);
}


//���������� � �����, ����� �� ��������� ������ � ������� ������������ �������
//�������, �� ����� ������ ��������� �������, ��� ��� � ������ ���� �������� ������� ������
//��� ���������, � ����� ��� ������������ ������ ������� ��� ��� ��� ���� �� ���������...

//������ ����������� ������ ������, �.�. � ������� �������� ������. ����� ��� ���� ������.
//����� ���������� ���� �����, �������� ���������� � ����� ������, ����� ������
class RuntimeSizedObjectsStack
{
    public:
        RuntimeSizedObjectsStack(size_t sizeBytes_arg = 1024) //������ ��������� ������
        :   internalData_m(new uint8_t[sizeBytes_arg])
            , bufferSize_m(sizeBytes_arg), stackTopShift_m(0)
        {}
        ~RuntimeSizedObjectsStack()
        {
            delete[] internalData_m;
        }


        bool isEmpty () const
        {
            return stackTopShift_m == 0;
        }
        void popWithoutGetting (size_t size_arg)
        {
            stackTopShift_m-=size_arg;
        }
        template<typename T>
        void popAndDestroy()
        {
            T* ptr = reinterpret_cast<T*>(getTopPointer());
            ptr->~T();//�������, ��� ������� ����������, � �� ������� ���� SomeType::~T()
            stackTopShift_m-=sizeof(T);
        }
        void pushMemory(uint8_t* ptrToData_arg, size_t size_arg)
        {
            stackTopShift_m += size_arg;
            if (stackTopShift_m>bufferSize_m)
                MoveToNewBuffer();

            SafeMemCopy(getTopPointer(), ptrToData_arg, size_arg);
        }
        template<typename T>
        void push(T& objToPush_arg)
        {
            stackTopShift_m += sizeof(T);
            if (stackTopShift_m>bufferSize_m)
                MoveToNewBuffer();

            new (getTopPointer())  T(objToPush_arg);
            //����������� new ������ ����� ������ T �� ������� �����
        }
        template<typename T>
        void push(T&& objToPush_arg)
        {
            stackTopShift_m += sizeof(T);
            if (stackTopShift_m>bufferSize_m)
                MoveToNewBuffer();

            new (getTopPointer())  T(objToPush_arg);
            //����������� new ������ ����� ������ T �� ������� �����
        }

        inline uint8_t* getTopPointer()
        {
            return ( (internalData_m)+bufferSize_m-stackTopShift_m);
        }
        void MoveToNewBuffer()
        {
            uint8_t* newData = new uint8_t[ (bufferSize_m*2) ];
            SafeMemCopy(newData-bufferSize_m, internalData_m, bufferSize_m);
            delete[] internalData_m;

            internalData_m = newData;
            bufferSize_m *= 2;
        }
    protected:
        uint8_t* internalData_m=nullptr;
        size_t bufferSize_m;
        size_t stackTopShift_m=0;


    private:
};

#endif // RUNTIMESIZEDOBJECTSSTACK_H
