#ifndef RANGE_H
#define RANGE_H

//говор€т, в range-base loop нельз€ запихать пару итераторов...
//но они ошибаютс€

template<typename Iter>
struct Range : public tuple<Iter, Iter>
{
    Range (Iter first_arg, Iter second_arg)
    :   tuple<Iter, Iter>(first_arg, second_arg)
    {

    }
    Iter begin()
    {
        return get<0>( static_cast<tuple<Iter, Iter>&>(*this) ); //ээ, а это точно так делаетс€? „то-то не очень красиво...
    }
    Iter end()
    {
        return get<1>( static_cast<tuple<Iter, Iter>&>(*this) );
    }
};
template<typename Iter>
Iter begin(const Range<Iter> & p )
{
    return get<0>(p);
}
template<typename Iter>
Iter end(const Range<Iter> & p )
{
    return get<1>(p);
}
template<typename Iter>
Range<Iter> make_range (Iter frst_arg, Iter scnd_arg)
{
    return Range<Iter>(frst_arg, scnd_arg);
}

#endif // RANGE_H
